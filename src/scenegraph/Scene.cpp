#include <scenegraph/Scene.h>
#include "SceneNode.h"

Scene::Scene() = default;
Scene::~Scene() = default;

SceneObject Scene::AddObject() noexcept {
	if (!_root) {
		_root = NewEntity<SceneNode>(Passkey{});
	}
	
	auto node = _root->AppendChildNode(NewEntity<SceneNode>(Passkey{}));
	return SceneObject{node};
}
