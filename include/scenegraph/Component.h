#pragma once

#include <scenegraph/linked/ForwardList.h>
#include <scenegraph/SceneEntity.h>

#include <type_traits>
#include <functional>

class SceneObject;

using ComponentType = uint32_t;

enum class ComponentMessage {
	Added,        // Added to SceneObject
	Removed,      // Removed from SceneObject
	Apply         // Applying to SceneObject
};

struct ComponentMessageParams {
	SceneObject* sceneObject;
};

///
/// Component composes a scene object
///
class Component : public ForwardListNode<>, public SceneEntity {
public:
	virtual ~Component() = default;
	
	virtual ComponentType Type() const noexcept = 0;
	
	template <typename T, typename = std::enable_if_t<std::is_base_of_v<Component, T>>>
	void DispatchMessagesTo(void (T::*mf)(ComponentMessage, ComponentMessageParams& params) noexcept) noexcept
		{ _dispatchMemFn = static_cast<DispatchMemFn>(mf); }
	
	void SendMessage(ComponentMessage message, ComponentMessageParams& params) noexcept
		{ std::invoke(_dispatchMemFn, this, message, params); }

	void Remove() noexcept { _removed = 1; }
	bool Removed() const noexcept { return _removed; }
	
protected:
	void DefaultDispatchMessage(ComponentMessage, ComponentMessageParams&) noexcept {}

private:
	using DispatchMemFn = void (Component::*)(ComponentMessage, ComponentMessageParams&) noexcept;

	DispatchMemFn _dispatchMemFn = &Component::DefaultDispatchMessage;
	
	uint8_t _removed    : 1 = 0;
//	uint8_t _reserved7  : 1 = 0;
//	uint8_t _reserved6  : 1 = 0;
//	uint8_t _reserved5  : 1 = 0;
//	uint8_t _reserved4  : 1 = 0;
//	uint8_t _reserved3  : 1 = 0;
//	uint8_t _reserved2  : 1 = 0;
//	uint8_t _reserved1  : 1 = 0;
};

///
///
///
template <typename T>
class ComponentImpl : public Component {
public:
	static std::unique_ptr<Component> Make(Scene* scene) noexcept;

protected:
	ComponentImpl() noexcept
	{
		DispatchMessagesTo(&ComponentImpl::DispatchMessage);
	}
	
private:
	void DispatchMessage(ComponentMessage message, ComponentMessageParams& params) noexcept {
		switch (message) {
		case ComponentMessage::Added:
			Derived()->Added(params.sceneObject);
			break;
		case ComponentMessage::Removed:
			Derived()->Removed(params.sceneObject);
			break;
		case ComponentMessage::Apply:
			Derived()->Apply(params.sceneObject);
			break;
		default:
			DefaultDispatchMessage(message, params);
			break;
		}
	}

	T* Derived() noexcept { return static_cast<T*>(this); }
	
	void Added(SceneObject*) noexcept {}
	void Removed(SceneObject*) noexcept {}
	void Apply(SceneObject*) noexcept {}
};

#define DEFINE_COMPONENT_TYPE(T) \
	static constexpr ComponentType kType = Murmur3Hash32(#T); \
	virtual ComponentType Type() const noexcept override { return kType; }

///
///
///
class ComponentList : public CircularForwardList<Component> {
public:
	void BroadcastMessage(ComponentMessage message, ComponentMessageParams& params) noexcept;
};
