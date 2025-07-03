#include <scenegraph/SceneObject.h>
#include <scenegraph/Component.h>
#include "SceneNode.h"

Scene* SceneObject::GetScene() noexcept {
	if (_node) {
		return _node->GetScene();
	}
	
	return nullptr;
}

Component* SceneObject::AddComponent(std::unique_ptr<Component> component) noexcept {
	assert(_node);
	
	_node->AddComponent(*component);
	
	component->SendMessage(Message::Added, this);
	
	return component.release();
}

// FindComponent

void SceneObject::BroadcastMessage(Message message) noexcept {
	if (!_node) {
		return;
	}
	
	auto& components = _node->GetComponents();
	
	components.BroadcastMessage(message, this);
}
