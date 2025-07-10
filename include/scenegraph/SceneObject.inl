#pragma once

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

template <typename T, typename Handler>
void SceneObject::ForEachComponent(Handler&& handler) noexcept {
	static_assert(std::is_base_of_v<ComponentImpl<T>, T>);
	
	ForEachComponent(T::kType,
		+[](SceneObject sceneObject, Component* component, bool& stop, void* context) {
			std::invoke(std::forward<Handler>(*static_cast<Handler*>(context)), sceneObject, static_cast<T*>(component), stop);
		},
		std::addressof(handler));
}

template <typename T, typename Handler>
void SceneObject::ForEachComponentInParent(Handler&& handler) noexcept {
	static_assert(std::is_base_of_v<ComponentImpl<T>, T>);
	
	ForEachComponentInParent(T::kType,
		+[](SceneObject sceneObject, Component* component, bool& stop, void* context) {
			std::invoke(std::forward<Handler>(*static_cast<Handler*>(context)), sceneObject, static_cast<T*>(component), stop);
		},
		std::addressof(handler));
}

template <typename T, typename Handler>
void SceneObject::ForEachComponentInChildren(Handler&& handler) noexcept {
	static_assert(std::is_base_of_v<ComponentImpl<T>, T>);
	
	ForEachComponentInChildren(T::kType,
		+[](SceneObject sceneObject, Component* component, bool& stop, void* context) {
			std::invoke(std::forward<Handler>(*static_cast<Handler*>(context)), sceneObject, static_cast<T*>(component), stop);
		},
		std::addressof(handler));
}

