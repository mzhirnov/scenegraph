#include <scenegraph/ComponentFactory.h>
#include <scenegraph/utils/MurmurHash.h>

void ComponentFactory::Register(std::string_view name, MakerType maker) {
	Register(Murmur3Hash32(name), maker);
}

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

std::unique_ptr<Component> ComponentFactory::MakeComponent(std::string_view name, Scene* scene) const noexcept {
	return MakeComponent(Murmur3Hash32(name), scene);
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
