#pragma once

#include <scenegraph/memory/MonotonicAllocator.h>
#include <scenegraph/utils/StaticImpl.h>
#include <scenegraph/utils/NonCopyable.h>
#include <scenegraph/SceneObject.h>
#include <scenegraph/Component.h>

#include <memory>
#include <type_traits>
#include <cstddef>

using SceneAllocator = MonotonicAllocator<1 << 14>;

///
/// Scene is a container of scene objects hierarchy
///
class Scene : public SceneAllocator {
public:
	using EnumObjectsCallback = void(*)(SceneObject sceneObject, bool& stop, void* context);
	
	Scene();
	~Scene();
	
	SceneObject AddObject() noexcept;
	
	class Passkey : NonCopyableNonMovable {
		friend class Scene;
		friend class SceneObject;
		template <typename T> friend class ComponentImpl;
		
		constexpr explicit Passkey() = default;
	};

	template <typename T, typename... Args>
	std::unique_ptr<T> NewEntity(Passkey, Args&&... args) noexcept;
	
	// void Handler(SceneObject sceneObject, bool& stop)
	template <typename Handler, typename = std::enable_if_t<std::is_invocable_v<Handler, SceneObject, bool&>>>
	bool ForEachObject(Handler&& handler) noexcept;
	
	bool ForEachObject(EnumObjectsCallback callback, void* context) noexcept;
	
private:
	// In order not to prevent disposing of first allocator's page by persistent root node,
	// don't use NewEntity and std::unique_ptr here.
	StaticImpl<SceneNode, 5 * sizeof(void*)> _root;
};

//---------------------------------------------------------------------------------------------------------------------

template <typename T, typename... Args>
std::unique_ptr<T> Scene::NewEntity(Passkey, Args&&... args) noexcept {
	static_assert(sizeof(T) > 0, "Type is not complete");
	static_assert(std::is_base_of_v<SceneEntity, T>);
	
	return std::unique_ptr<T>(std::construct_at(Allocate<T>(), std::forward<Args>(args)...));
}

template <typename Handler, typename>
bool Scene::ForEachObject(Handler&& handler) noexcept {
	return ForEachObject(
		+[](SceneObject sceneObject, bool& stop, void* context) {
			std::invoke(std::forward<Handler>(*static_cast<Handler*>(context)), sceneObject, stop);
		},
		std::addressof(handler));
}

//---------------------------------------------------------------------------------------------------------------------

template <typename T>
T* SceneObject::AddComponent() noexcept {
	static_assert(std::is_base_of_v<ComponentImpl<T>, T>);
	
	if (auto scene = GetScene()) {
		return static_cast<T*>(AddComponent(T::Make(scene)));
	}
	
	return nullptr;
}

template <typename T>
T* SceneObject::FindComponent() noexcept {
	static_assert(std::is_base_of_v<ComponentImpl<T>, T>);
	
	return static_cast<T*>(FindComponent(T::kType));
}

template <typename T>
T* SceneObject::FindComponentInParent() noexcept {
	static_assert(std::is_base_of_v<ComponentImpl<T>, T>);
	
	return static_cast<T*>(FindComponentInParent(T::kType));
}

template <typename T>
T* SceneObject::FindComponentInChildren() noexcept {
	static_assert(std::is_base_of_v<ComponentImpl<T>, T>);
	
	return static_cast<T*>(FindComponentInChildren(T::kType));
}

//---------------------------------------------------------------------------------------------------------------------

template <typename T>
std::unique_ptr<Component> ComponentImpl<T>::Make(Scene* scene) noexcept {
	return scene->NewEntity<T>(Scene::Passkey{});
}
