#include <scenegraph/Scene.h>
#include "SceneNode.h"

Scene::Scene() = default;
Scene::~Scene() = default;

SceneObject Scene::AddObject() noexcept {
	return SceneObject{_root->AppendChildNode(NewEntity<SceneNode>(Passkey{}))};
}
