#pragma once

#include <memory>

class Scene;
class SceneNode;
class Component;

enum class ComponentMessage;

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

	SceneObject(const SceneObject&) = delete;
	SceneObject& operator=(const SceneObject&) = delete;
	
	bool operator==(const SceneObject& rhs) const noexcept { return _node == rhs._node; }
	bool operator<(const SceneObject& rhs) const noexcept { return _node < rhs._node; }
	
	bool operator!() const noexcept { return !_node; }
	explicit operator bool() const noexcept { return _node != nullptr; }
	
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
	
	Scene* GetScene() noexcept;
	
	Component* AddComponent(std::unique_ptr<Component> component) noexcept;
	
	template <typename T>
	T* AddComponent() noexcept;

	// T* FindComponent<T>() noexcept
	
	void SendMessage(ComponentMessage message) noexcept;

private:
	SceneNode* _node = nullptr;
};
