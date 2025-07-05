#include <scenegraph/SceneObject.h>
#include <scenegraph/Component.h>
#include "SceneNode.h"

SceneObject SceneObject::Parent() const noexcept {
	return SceneObject{_node ? _node->GetParentNode() : nullptr};
}

SceneObject SceneObject::FirstChild() const noexcept {
	return SceneObject{_node ? _node->GetFirstChildNode() : nullptr};
}

SceneObject SceneObject::LastChild() const noexcept {
	return SceneObject{_node ? _node->GetLastChildNode() : nullptr};
}

SceneObject SceneObject::ChildNodeAt(int pos) const noexcept {
	return SceneObject{_node ? _node->GetChildNodeAt(pos) : nullptr};
}

SceneObject SceneObject::NextSibling() const noexcept {
	return SceneObject{_node ? _node->GetNextSiblingNode() : nullptr};
}

SceneObject SceneObject::PrevSibling() const noexcept {
	return SceneObject{_node ? _node->GetPrevSiblingNode() : nullptr};
}

SceneObject SceneObject::AppendChild() noexcept {
	return SceneObject{_node ? _node->AppendChildNode(GetScene()->NewEntity<SceneNode>(Scene::Passkey{})) : nullptr};
}

SceneObject SceneObject::PrependChild() noexcept {
	return SceneObject{_node ? _node->PrependChildNode(GetScene()->NewEntity<SceneNode>(Scene::Passkey{})) : nullptr};
}

SceneObject SceneObject::InsertChildAt(int pos) noexcept {
	return SceneObject{_node ? _node->InsertChildNodeAt(GetScene()->NewEntity<SceneNode>(Scene::Passkey{}), pos) : nullptr};
}

SceneObject SceneObject::InsertAfter() noexcept {
	return SceneObject{_node ? _node->InsertNodeAfter(GetScene()->NewEntity<SceneNode>(Scene::Passkey{})) : nullptr};
}

SceneObject SceneObject::InsertBefore() noexcept {
	return SceneObject{_node ? _node->InsertNodeBefore(GetScene()->NewEntity<SceneNode>(Scene::Passkey{})) : nullptr};
}

void SceneObject::RemoveChildAt(int pos) noexcept {
	if (_node) {
		_node->RemoveChildNodeAt(pos);
	}
}

void SceneObject::RemoveChildren() noexcept {
	if (_node) {
		_node->RemoveAllChildNodes();
	}
}

void SceneObject::RemoveFromParent() noexcept {
	if (_node) {
		_node->RemoveFromParent();
	}
}

Scene* SceneObject::GetScene() noexcept {
	return _node ? _node->GetScene() : nullptr;
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
