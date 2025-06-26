#pragma once

#include <scenegraph/Hierarchy.h>
#include <scenegraph/SceneEntity.h>
#include <scenegraph/Component.h>

///
/// Scene object is a building block of scene hierarchy
///
class SceneObject : private Hierarchy<SceneObject>, public SceneEntity {
public:
	SceneObject() = default;

	SceneObject(const SceneObject&) = delete;
	SceneObject& operator=(const SceneObject&) = delete;

	void AddComponent(Component& c) noexcept {
		_components.Add(c);
		c.SendMessage(Component::Message::Added, this);
	}

	// FindComponent
	
	void Update() noexcept { _components.BroadcastMessage(Component::Message::Apply, this); }

private:
	friend class Hierarchy<SceneObject>;
	
	ComponentList _components;
	//BehaviorList _behaviors;
	//AttributeList _attributes;
};
