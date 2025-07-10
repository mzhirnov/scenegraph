#include <scenegraph/ComponentFactory.h>
#include <scenegraph/Component.h>

void DynamicFactoryPolicy::Register(ComponentType type, ComponentMakerType maker) {
#ifndef NDEBUG
	bool duplicateFound = false;
	for (auto& [hash, _] : _makers) {
		if (hash == type) {
			duplicateFound = true;
			break;
		}
	}
	assert(!duplicateFound);
#endif

	_makers.emplace_back(type, maker);
}

std::unique_ptr<Component> DynamicFactoryPolicy::MakeComponent(ComponentType type, Scene* scene) const noexcept {
	for (auto& [hash, maker] : _makers) {
		if (hash == type) {
			return maker(scene);
		}
	}
	
	return {};
}
