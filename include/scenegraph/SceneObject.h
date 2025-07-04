#pragma once

#include <scenegraph/linked/Hierarchy.h>
#include <scenegraph/SceneEntity.h>

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
		assert(node);
	}

	SceneObject(const SceneObject&) = delete;
	SceneObject& operator=(const SceneObject&) = delete;
	
	bool operator==(const SceneObject& rhs) const noexcept { return _node == rhs._node; }
	bool operator<(const SceneObject& rhs) const noexcept { return _node < rhs._node; }
	
	bool operator!() const noexcept { return !_node; }
	explicit operator bool() const noexcept { return _node != nullptr; }
	
	Scene* GetScene() noexcept;
	
	Component* AddComponent(std::unique_ptr<Component> component) noexcept;
	
	template <typename T>
	T* AddComponent() noexcept;

	// T* FindComponent<T>() noexcept
	
	void SendMessage(ComponentMessage message) noexcept;

private:
	SceneNode* _node = nullptr;
};
