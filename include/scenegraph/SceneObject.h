#pragma once

#include <scenegraph/ForwardList.h>

#include <type_traits>
#include <functional>

class SceneObject;

///
///
///
class Component : public ForwardListNode<> {
public:
	enum class Message;
	
	template <typename T, typename = std::enable_if_t<std::is_base_of_v<Component, T>>>
	void DispatchMessagesTo(void (T::*mf)(Message, SceneObject*) noexcept) noexcept
		{ _dispatchMemFn = static_cast<DispatchMemFn>(mf); }
	
	void SendMessage(Message message, SceneObject* sceneObject) noexcept
		{ _dispatchMemFn ? std::invoke(_dispatchMemFn, this, message, sceneObject) : static_cast<void>(0); }

	bool Valid() const noexcept { return !!_dispatchMemFn; }
	void Invalidate() noexcept { _dispatchMemFn = {}; }

private:
	using DispatchMemFn = void (Component::*)(Message, SceneObject*) noexcept;

	DispatchMemFn _dispatchMemFn{};
};

enum class Component::Message {
	Apply
};

///
///
///
class ComponentList : public CircularForwardList<Component> {
public:
	void DispatchMessage(Component::Message message, SceneObject* sceneObject) noexcept {
		for (auto it = begin(), e = end(); it != e; /**/) {
			if ((*it).Valid()) {
				(*it).SendMessage(message, sceneObject);
				++it;
			}
			else {
				it = Erase(it);
			}
		}
	}
};

///
///
///
class SceneObject {
public:
	SceneObject() = default;

	SceneObject(const SceneObject&) = delete;
	SceneObject& operator=(const SceneObject&) = delete;

	void AddComponent(Component& c) noexcept { _components.PushBack(c); }

	// FindComponent
	
	void Update() noexcept { _components.DispatchMessage(Component::Message::Apply, this); }

private:
	ComponentList _components;
	//BehaviorList _behaviors;
	//AttributeList _attributes;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename T>
class ComponentImpl : public Component {
public:
	ComponentImpl() noexcept { DispatchMessagesTo(&ComponentImpl::DispatchMessageImpl); }

private:
	void DispatchMessageImpl(Component::Message message, SceneObject* sceneObject) noexcept {
		switch (message) {
		case Component::Message::Apply:
			Derived()->Apply(sceneObject);
			break;
		}
	}

	T* Derived() noexcept { return static_cast<T*>(this); }

	void Apply(SceneObject*) noexcept {}
};
