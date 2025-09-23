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
	SceneNode* GetNode() noexcept { return _node; }
	
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
	
	// void Handler(SceneObject, bool& stop)
	template <typename Handler, typename = std::enable_if_t<std::is_invocable_v<Handler, SceneObject, bool&>>>
	bool ForEachObjectInParent(Handler&& handler) noexcept;
	
	template <typename Handler, typename = std::enable_if_t<std::is_invocable_v<Handler, SceneObject, bool&>>>
	bool ForEachObjectInChildren(Handler&& handler) noexcept;
	
	template <typename Handler, typename = std::enable_if_t<std::is_invocable_v<Handler, SceneObject, EnumCallOrder, bool&>>>
	bool WalkChildren(EnumDirection direction, EnumCallOrder callOrder, Handler&& handler) noexcept;
	
	// C o m p o n e n t s
	
	Component* AddComponent(std::unique_ptr<Component> component) noexcept;
	
	template <typename T> T* AddComponent() noexcept;
	
	template <typename T> T* FindComponent() noexcept;
	template <typename T> T* FindComponentInParent() noexcept;
	template <typename T> T* FindComponentInChildren() noexcept;
	
	// void Handler(SceneObject, T* component, bool& stop)
	template <typename T, typename Handler, typename = std::enable_if_t<std::is_invocable_v<Handler, SceneObject, T*, bool&>>>
	bool ForEachComponent(Handler&& handler) noexcept;
	
	template <typename T, typename Handler, typename = std::enable_if_t<std::is_invocable_v<Handler, SceneObject, T*, bool&>>>
	bool ForEachComponentInParent(Handler&& handler) noexcept;
	
	template <typename T, typename Handler, typename = std::enable_if_t<std::is_invocable_v<Handler, SceneObject, T*, bool&>>>
	bool ForEachComponentInChildren(Handler&& handler) noexcept;
	
	// Sends message to own components
	void SendMessage(ComponentMessage message, ComponentMessageParams& params) noexcept;
	// Sends message to all parent components
	void SendMessageInParent(ComponentMessage message, ComponentMessageParams& params) noexcept;
	// Sends message to all children components
	void SendMessageInChildren(ComponentMessage message, ComponentMessageParams& params) noexcept;
	// Sends message to own and all children components
	void BroadcastMessage(ComponentMessage message, ComponentMessageParams& params) noexcept;
	
private:
	using EnumObjectsCallback = void(*)(SceneObject sceneObject, bool& stop, void* context);
	using WalkObjectsCallback = void(*)(SceneObject sceneObject, EnumCallOrder callOrder, bool& stop, void* context);
	using EnumComponentsCallback = void(*)(SceneObject sceneObject, Component* component, bool& stop, void* context);
	
	bool ForEachObjectInParent(EnumObjectsCallback callback, void* context) noexcept;
	bool ForEachObjectInChildren(EnumObjectsCallback callback, void* context) noexcept;
	
	bool WalkChildren(EnumDirection direction, EnumCallOrder callOrder, WalkObjectsCallback callback, void* context) noexcept;
	
	Component* FindComponent(ComponentType type) noexcept;
	Component* FindComponentInParent(ComponentType type) noexcept;
	Component* FindComponentInChildren(ComponentType type) noexcept;
	
	bool ForEachComponent(ComponentType type, EnumComponentsCallback callback, void* context) noexcept;
	bool ForEachComponentInParent(ComponentType type, EnumComponentsCallback callback, void* context) noexcept;
	bool ForEachComponentInChildren(ComponentType type, EnumComponentsCallback callback, void* context) noexcept;

private:
	SceneNode* _node = nullptr;
};
