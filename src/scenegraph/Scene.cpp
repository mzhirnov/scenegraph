#include <scenegraph/Scene.h>
#include "SceneNode.h"

Scene::Scene() = default;

Scene::~Scene() = default;

SceneObject Scene::AddObject() noexcept {
	return SceneObject{_root->AppendChildNode(NewEntity<SceneNode>(Passkey{}))};
}

bool Scene::ForEachObject(EnumObjectsCallback callback, void* context) noexcept {
	if (!callback) {
		return false;
	}
	
	bool stop = false;
	
	for (auto node = _root->GetFirstChildNode(); node && !stop; node = node->GetNextSiblingNode()) {
		callback(SceneObject{node}, stop, context);
	}
	
	return stop;
}
