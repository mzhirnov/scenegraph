#pragma once

#include <scenegraph/linked/ForwardList.h>
#include <scenegraph/SceneEntity.h>

#include <type_traits>
#include <functional>

class SceneObject;

enum class Message {
	Added,
	Removed,
	Apply
};

///
/// Component composes a scene object
///
class Component : public ForwardListNode<>, public SceneEntity {
public:
	virtual ~Component() = default;
	
	const std::type_info& Type() const noexcept { return typeid(*this); }
	
	template <typename T, typename = std::enable_if_t<std::is_base_of_v<Component, T>>>
	void DispatchMessagesTo(void (T::*mf)(Message, SceneObject*) noexcept) noexcept
		{ _dispatchMemFn = static_cast<DispatchMemFn>(mf); }
	
	void SendMessage(Message message, SceneObject* sceneObject) noexcept
		{ std::invoke(_dispatchMemFn, this, message, sceneObject); }

	void Remove() noexcept { _removed = true; }
	bool Removed() const noexcept { return _removed; }
	
	void DefaultDispatchMessage(Message, SceneObject*) noexcept {}

private:
	using DispatchMemFn = void (Component::*)(Message, SceneObject*) noexcept;

	DispatchMemFn _dispatchMemFn = &Component::DefaultDispatchMessage;
	
	bool _removed = false;
};

///
///
///
template <typename T>
class ComponentImpl : public Component {
public:
	static constexpr const std::type_info& StaticType() noexcept { return typeid(T); }
	static std::unique_ptr<T> Make(Scene* scene) noexcept;
	
	ComponentImpl() noexcept
	{
		DispatchMessagesTo(&ComponentImpl::DispatchMessage);
	}

private:
	void DispatchMessage(Message message, SceneObject* sceneObject) noexcept {
		switch (message) {
		case Message::Added:
			Derived()->Added(sceneObject);
			break;
		case Message::Removed:
			Derived()->Removed(sceneObject);
			break;
		case Message::Apply:
			Derived()->Apply(sceneObject);
			break;
		default:
			DefaultDispatchMessage(message, sceneObject);
			break;
		}
	}

	T* Derived() noexcept { return static_cast<T*>(this); }
	
	void Added(SceneObject*) noexcept {}
	void Removed(SceneObject*) noexcept {}
	void Apply(SceneObject*) noexcept {}
};

///
///
///
class ComponentList : public CircularForwardList<Component> {
public:
	void Add(Component& c) {
		PushBack(c);
	}
	
	void BroadcastMessage(Message message, SceneObject* sceneObject) noexcept {
		for (auto it = begin(), e = end(); it != e; /**/) {
			if (auto& component = *it; !component.Removed()) {
				component.SendMessage(message, sceneObject);
				++it;
			}
			else {
				component.SendMessage(Message::Removed, sceneObject);
				it = Erase(it);
				delete std::addressof(component);
			}
		}
	}
};
