#pragma once

#include <scenegraph/ComponentTypes.h>

#include <memory>

class Scene;
class SceneNode;
class Component;

///
/// Scene object is a building block of scene hierarchy
///
class SceneObject {
public:
	SceneObject() = default;
	
	explicit SceneObject(SceneNode* node) noexcept
		: _node(node)
	{
	}
	
	bool operator==(const SceneObject& rhs) const noexcept { return _node == rhs._node; }
	bool operator<(const SceneObject& rhs) const noexcept { return _node < rhs._node; }
	
	bool operator!() const noexcept { return !_node; }
	explicit operator bool() const noexcept { return _node != nullptr; }
	
	Scene* GetScene() noexcept;
	
	// H i e r a r c h y
	
	SceneObject Parent() const noexcept;
	SceneObject FirstChild() const noexcept;
	SceneObject LastChild() const noexcept;
	SceneObject ChildNodeAt(int pos) const noexcept;
	SceneObject NextSibling() const noexcept;
	SceneObject PrevSibling() const noexcept;
	
	SceneObject AppendChild() noexcept;
	SceneObject PrependChild() noexcept;
	SceneObject InsertChildAt(int pos) noexcept;
	SceneObject InsertAfter() noexcept;
	SceneObject InsertBefore() noexcept;
	
	void RemoveChildAt(int pos) noexcept;
	void RemoveChildren() noexcept;
	void RemoveFromParent() noexcept;
	
	// C o m p o n e n t s
	
	using EnumComponentsCallback = void(*)(SceneObject sceneObject, Component* component, bool& stop, void* context);
	
	Component* AddComponent(std::unique_ptr<Component> component) noexcept;
	
	template <typename T> T* AddComponent() noexcept;

	Component* FindComponent(ComponentType type) noexcept;
	Component* FindComponentInParent(ComponentType type) noexcept;
	Component* FindComponentInChildren(ComponentType type) noexcept;
	
	void ForEachComponent(ComponentType type, EnumComponentsCallback callback, void* context) noexcept;
	void ForEachComponentInParent(ComponentType type, EnumComponentsCallback callback, void* context) noexcept;
	void ForEachComponentInChildren(ComponentType type, EnumComponentsCallback callback, void* context) noexcept;
	
	template <typename T> T* FindComponent() noexcept;
	template <typename T> T* FindComponentInParent() noexcept;
	template <typename T> T* FindComponentInChildren() noexcept;
	
	// void Handler(SceneObject, T* component, bool& stop)
	template <typename T, typename Handler, typename = std::enable_if_t<std::is_invocable_v<Handler, SceneObject, T*, bool&>>>
	void ForEachComponent(Handler&& handler) noexcept;
	
	template <typename T, typename Handler, typename = std::enable_if_t<std::is_invocable_v<Handler, SceneObject, T*, bool&>>>
	void ForEachComponentInParent(Handler&& handler) noexcept;
	
	template <typename T, typename Handler, typename = std::enable_if_t<std::is_invocable_v<Handler, SceneObject, T*, bool&>>>
	void ForEachComponentInChildren(Handler&& handler) noexcept;
	
	// Sends message to own components
	void SendMessage(ComponentMessage message, ComponentMessageParams& params) noexcept;
	// Sends message to own components and those in all children hierarchy
	void BroadcastMessage(ComponentMessage message, ComponentMessageParams& params) noexcept;

private:
	SceneNode* _node = nullptr;
};
