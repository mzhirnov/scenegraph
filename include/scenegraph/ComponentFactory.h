#pragma once

#include <scenegraph/utils/MurmurHash.h>

#include <memory>
#include <string_view>
#include <vector>

class Scene;
class Component;

///
///
///
class ComponentFactory {
public:
	using HashType = uint32_t;
	using MakerType = std::unique_ptr<Component> (*)(Scene* scene) noexcept;
	
	constexpr static HashType HashName(std::string_view name) noexcept { return Murmur3Hash32(name); }
	
	ComponentFactory() = default;
	
	void Register(HashType hashedName, MakerType maker);
	void Register(std::string_view name, MakerType maker)
		{ Register(HashName(name), maker); }
	
	std::unique_ptr<Component> MakeComponent(HashType hashedName, Scene* scene) const noexcept;
	std::unique_ptr<Component> MakeComponent(std::string_view name, Scene* scene) const noexcept
		{ return MakeComponent(HashName(name), scene); }
	
private:
	std::vector<std::pair<HashType, MakerType>> _makers;
};
