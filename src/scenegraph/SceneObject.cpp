#include <scenegraph/SceneObject.h>
#include <scenegraph/Component.h>
#include "SceneNode.h"

Scene* SceneObject::GetScene() noexcept {
	return _node ? _node->GetScene() : nullptr;
}

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

Component* SceneObject::FindComponent(ComponentType type) noexcept {
	if (!_node) {
		return nullptr;
	}
	
	for (auto& component : _node->GetComponents()) {
		if (component.Type() == type) {
			return std::addressof(component);
		}
	}
	
	return nullptr;
}

Component* SceneObject::FindComponentInParent(ComponentType type) noexcept {
	if (!_node) {
		return nullptr;
	}
	
	for (auto parent = _node->GetParentNode(); parent; parent = parent->GetParentNode()) {
		if (auto component = SceneObject{parent}.FindComponent(type)) {
			return component;
		}
	}
	
	return nullptr;
}

Component* SceneObject::FindComponentInChildren(ComponentType type) noexcept {
	if (!_node) {
		return nullptr;
	}
	
	Component* component = nullptr;
	
	_node->ForEachChildNode(EnumDirection::FirstToLast, EnumCallOrder::PreOrder,
		[type, &component](EnumCallOrder, SceneNode* node, bool& stop) {
			if ((component = SceneObject{node}.FindComponent(type))) {
				stop = true;
			}
		});
	
	return component;
}

void SceneObject::ForEachComponent(ComponentType type, EnumComponentsCallback callback, void* context) noexcept {
	if (!_node || !callback) {
		return;
	}
	
	bool stop = false;
	
	for (auto& component : _node->GetComponents()) {
		if (component.Type() == type) {
			callback(*this, std::addressof(component), stop, context);
			if (stop) {
				break;
			}
		}
	}
}

void SceneObject::ForEachComponentInParent(ComponentType type, EnumComponentsCallback callback, void* context) noexcept {
	if (!_node || !callback) {
		return;
	}
	
	bool stop = false;
	
	for (auto parent = _node->GetParentNode(); parent && !stop; parent = parent->GetParentNode()) {
		SceneObject sceneObject{parent};
		for (auto& component : parent->GetComponents()) {
			if (component.Type() == type) {
				callback(sceneObject, std::addressof(component), stop, context);
			}
		}
	}
}

void SceneObject::ForEachComponentInChildren(ComponentType type, EnumComponentsCallback callback, void* context) noexcept {
	if (!_node || !callback) {
		return;
	}
	
	_node->ForEachChildNode(EnumDirection::FirstToLast, EnumCallOrder::PreOrder,
		[type, callback, context](EnumCallOrder, SceneNode* node, bool& stop) {
			SceneObject sceneObject{node};
			for (auto& component : node->GetComponents()) {
				if (component.Type() == type) {
					callback(sceneObject, std::addressof(component), stop, context);
				}
			}
		});
}


void SceneObject::SendMessage(ComponentMessage message, ComponentMessageParams& params) noexcept {
	if (!_node) {
		return;
	}
	
	params.sceneObject = this;
	
	auto& components = _node->GetComponents();
	components.BroadcastMessage(message, params);
}

void SceneObject::BroadcastMessage(ComponentMessage message, ComponentMessageParams& params) noexcept {
	if (!_node) {
		return;
	}
	
	params.sceneObject = this;
	auto& components = _node->GetComponents();
	components.BroadcastMessage(message, params);
	
	_node->ForEachChildNode(EnumDirection::FirstToLast, EnumCallOrder::PreOrder,
		[message, &params](EnumCallOrder, SceneNode* node, bool&) {
			SceneObject sceneObject{node};
			params.sceneObject = &sceneObject;
			
			auto& components = node->GetComponents();
			components.BroadcastMessage(message, params);
		});
}
