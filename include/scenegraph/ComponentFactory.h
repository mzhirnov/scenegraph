#pragma once

#include <scenegraph/Component.h>

#include <memory>
#include <string_view>
#include <vector>

class Scene;

///
///
///
class ComponentFactory {
public:
	using HashType = uint32_t;
	using MakerType = std::unique_ptr<Component> (*)(Scene* scene) noexcept;
	
	ComponentFactory() = default;
	
	void Register(std::string_view name, MakerType maker);
	void Register(HashType hashedName, MakerType maker);
	
	std::unique_ptr<Component> MakeComponent(std::string_view name, Scene* scene) const noexcept;
	std::unique_ptr<Component> MakeComponent(HashType hashedName, Scene* scene) const noexcept;
	
private:
	std::vector<std::pair<HashType, MakerType>> _makers;
};
