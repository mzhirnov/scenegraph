#pragma once

#include <scenegraph/utils/MurmurHash.h>

#include <memory>
#include <string_view>
#include <vector>

class Scene;
class Component;
template <typename> class ComponentImpl;

using HashType = uint32_t;
using MakerType = std::unique_ptr<Component> (*)(Scene* scene) noexcept;

///
///
///
template <typename Policy>
class ComponentFactory : public Policy {
public:
	constexpr static HashType HashName(std::string_view name) noexcept { return Murmur3Hash32(name); }
	
	ComponentFactory() = default;
	
	using Policy::MakeComponent;
	
	std::unique_ptr<Component> MakeComponent(std::string_view name, Scene* scene) const noexcept
		{ return MakeComponent(Murmur3Hash32(name), scene); }
};

///
///
///
class DynamicFactoryPolicy {
public:
	DynamicFactoryPolicy() = default;
	
	template <typename C>
	void Register() {
		static_assert(std::is_base_of_v<ComponentImpl<C>, C>);
		Register(C::Type, C::Make);
	}
	
	void Register(std::string_view name, MakerType maker)
		{ Register(Murmur3Hash32(name), maker); }
	
	void Register(HashType hashedName, MakerType maker);
	
	std::unique_ptr<Component> MakeComponent(HashType hashedName, Scene* scene) const noexcept;
	
protected:
	std::vector<std::pair<HashType, MakerType>> _makers;
};

///
///
///
template <typename... Ts>
class ComponentTypeList {
public:
	static_assert((std::is_base_of_v<Component, Ts> && ...));
	
	using ArrayType = std::array<std::pair<HashType, MakerType>, sizeof...(Ts)>;
	
	static constexpr auto ToSortedArray() noexcept {
		ArrayType arr = { std::make_pair(Ts::Type, Ts::Make)... };
		std::sort(arr.begin(), arr.end(), [](auto& lhs, auto& rhs) { return lhs.first < rhs.first; });
		return arr;
	}
};

///
///
///
template <typename CL>
class StaticFactoryPolicy {
public:
	StaticFactoryPolicy() = default;
	
	std::unique_ptr<Component> MakeComponent(HashType hashedName, Scene* scene) const noexcept {
		auto it = std::lower_bound(_makers.begin(), _makers.end(), hashedName,
			[](const auto& elem, auto value) { return elem.first < value; });
		
		if (it != _makers.end() && (*it).first == hashedName) {
			return (*it).second(scene);
		}
		
		return {};
	}

protected:
	typename CL::ArrayType _makers = CL::ToSortedArray();
};
