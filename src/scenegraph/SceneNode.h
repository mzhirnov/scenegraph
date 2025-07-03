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
	
	void AddComponent(Component& c) noexcept { _components.Add(c); }
	
	// T* FindComponent<T>() noexcept
	
	ComponentList& GetComponents() noexcept { return _components; }
	
private:
	friend class Hierarchy<SceneNode>;
	
	ComponentList _components;
	//BehaviorList _behaviors;
	//AttributeList _attributes;
};
