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
	assert(component);
	
	if (!_node || !component) {
		return nullptr;
	}
	
	_node->AddComponent(*component);
	
	ComponentMessageParams params { this };
	component->SendMessage(ComponentMessage::Added, params);
	
	return component.release();
}

// FindComponent

void SceneObject::SendMessage(ComponentMessage message) noexcept {
	if (!_node) {
		return;
	}
	
	auto& components = _node->GetComponents();
	
	ComponentMessageParams params { this };
	components.BroadcastMessage(message, params);
}
