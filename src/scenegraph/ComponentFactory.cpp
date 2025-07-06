#include <scenegraph/ComponentFactory.h>
#include <scenegraph/Component.h>

void DynamicFactoryPolicy::Register(HashType hashedName, MakerType maker) {
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

std::unique_ptr<Component> DynamicFactoryPolicy::MakeComponent(HashType hashedName, Scene* scene) const noexcept {
	for (auto& [hash, maker] : this->_makers) {
		if (hash == hashedName) {
			return maker(scene);
		}
	}
	
	return {};
}
