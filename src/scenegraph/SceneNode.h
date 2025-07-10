#pragma once

#include <scenegraph/linked/Hierarchy.h>
#include <scenegraph/SceneEntity.h>
#include <scenegraph/Component.h>
#include <scenegraph/Scene.h>

///
/// Scene node is a node of scene hierarchy
///
class SceneNode : public Hierarchy<SceneNode>, public SceneEntity {
public:
	SceneNode() = default;
	~SceneNode();
	
	SceneNode(const SceneNode&) = delete;
	SceneNode& operator=(const SceneNode&) = delete;
	
	ComponentList components;
	
	void AddComponent(Component& c) noexcept { components.PushBack(c); }
};
