#pragma once

#include <scenegraph/ComponentTypes.h>

#include <memory>
#include <vector>

class Scene;
class Component;
template <typename> class ComponentImpl;

using ComponentMakerType = std::unique_ptr<Component> (*)(Scene* scene) noexcept;

///
///
///
template <typename Policy>
class ComponentFactory : public Policy {
public:
	ComponentFactory() = default;
	
	using Policy::MakeComponent;
	
	std::unique_ptr<Component> MakeComponent(std::string_view name, Scene* scene) const noexcept
		{ return MakeComponent(ComponentTypeFromName(name), scene); }
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
		Register(C::kType, C::Make);
	}
	
	void Register(std::string_view name, ComponentMakerType maker)
		{ Register(ComponentTypeFromName(name), maker); }
	
	void Register(ComponentType type, ComponentMakerType maker);
	
	std::unique_ptr<Component> MakeComponent(ComponentType type, Scene* scene) const noexcept;
	
protected:
	std::vector<std::pair<ComponentType, ComponentMakerType>> _makers;
};

///
///
///
template <typename... Ts>
class ComponentTypeList {
public:
	static_assert((std::is_base_of_v<Component, Ts> && ...));
	
	using ArrayType = std::array<std::pair<ComponentType, ComponentMakerType>, sizeof...(Ts)>;
	
	static constexpr auto ToSortedArray() noexcept {
		ArrayType arr = { std::make_pair(Ts::kType, Ts::Make)... };
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
	
	std::unique_ptr<Component> MakeComponent(ComponentType type, Scene* scene) const noexcept {
		auto it = std::lower_bound(_makers.begin(), _makers.end(), type,
			[](const auto& elem, auto value) { return elem.first < value; });
		
		if (it != _makers.end() && (*it).first == type) {
			return (*it).second(scene);
		}
		
		return {};
	}

protected:
	typename CL::ArrayType _makers = CL::ToSortedArray();
};
