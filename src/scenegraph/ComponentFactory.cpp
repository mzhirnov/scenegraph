#include <scenegraph/ComponentFactory.h>
#include <scenegraph/Component.h>

void ComponentFactory::Register(HashType hashedName, MakerType maker) {
#ifndef NDEBUG
	bool duplicateFound = false;
	for (auto& [hash, _] : _makers) {
		if (hash == hashedName) {
			duplicateFound = true;
			break;
		}
	}
	assert(!duplicateFound);
#endif
	
	_makers.emplace_back(hashedName, maker);
}

std::unique_ptr<Component> ComponentFactory::MakeComponent(HashType hashedName, Scene* scene) const noexcept {
	for (auto& [hash, maker] : _makers) {
		if (hash == hashedName) {
			return maker(scene);
		}
	}
	
	assert(false && "Component not found");
	return {};
}
