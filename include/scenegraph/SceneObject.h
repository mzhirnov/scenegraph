#pragma once

#include <memory>

class Scene;
class SceneNode;
class Component;

enum class ComponentMessage;
struct ComponentMessageParams;

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
	
	Component* AddComponent(std::unique_ptr<Component> component) noexcept;
	
	template <typename T> T* AddComponent() noexcept;

	Component* FindComponent(uint32_t type) noexcept;
	Component* FindComponentInParent(uint32_t type) noexcept;
	Component* FindComponentInChildren(uint32_t type) noexcept;
	
	template <typename T> T* FindComponent() noexcept;
	template <typename T> T* FindComponentInParent() noexcept;
	template <typename T> T* FindComponentInChildren() noexcept;
	
	// Sends message to own components
	void SendMessage(ComponentMessage message, ComponentMessageParams& params) noexcept;
	// Sends message to own components and those in all children hierarchy
	void BroadcastMessage(ComponentMessage message, ComponentMessageParams& params) noexcept;

private:
	SceneNode* _node = nullptr;
};
