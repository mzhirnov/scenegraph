#include <scenegraph/Scene.h>
#include "SceneNode.h"

Scene::Scene() = default;

Scene::~Scene() {
	// Destroy list inplace in the loop instead of auto recursion
	while (nextScene) {
		nextScene = std::move(nextScene->nextScene);
	}
}

std::unique_ptr<SceneString> Scene::NewString(std::string_view str) noexcept {
	auto p = static_cast<char*>(Allocate(str.size() + 1, 1));
	std::memcpy(p, str.data(), str.size());
	p[str.size()] = 0;
	return std::unique_ptr<SceneString>(static_cast<SceneString*>(static_cast<void*>(p)));
}

SceneObject Scene::AddObject() noexcept {
	return SceneObject{_root->AppendChildNode(NewEntity<SceneNode>(Passkey{}))};
}

bool Scene::ForEachRootObject(EnumObjectsCallback callback, void* context) noexcept {
	if (!callback) {
		return false;
	}
	
	bool stop = false;
	
	for (auto node = _root->GetFirstChildNode(); node && !stop; node = node->GetNextSiblingNode()) {
		callback(SceneObject{node}, stop, context);
	}
	
	return stop;
}
