#pragma once

template <typename Handler, typename>
bool SceneObject::ForEachObjectInParent(Handler&& handler) noexcept {
	return ForEachObjectInParent(
		+[](SceneObject sceneObject, bool& stop, void* context) {
			std::invoke(std::forward<Handler>(*static_cast<Handler*>(context)), sceneObject, stop);
		},
		std::addressof(handler));
}

template <typename Handler, typename>
bool SceneObject::ForEachObjectInChildren(Handler&& handler) noexcept {
	return ForEachObjectInChildren(
		+[](SceneObject sceneObject, bool& stop, void* context) {
			std::invoke(std::forward<Handler>(*static_cast<Handler*>(context)), sceneObject, stop);
		},
		std::addressof(handler));
}

template <typename Handler, typename>
bool SceneObject::WalkChildren(EnumDirection direction, EnumCallOrder callOrder, Handler&& handler) noexcept {
	return WalkChildren(direction, callOrder,
		+[](SceneObject sceneObject, EnumCallOrder callOrder, bool& stop, void* context) {
			std::invoke(std::forward<Handler>(*static_cast<Handler*>(context)), sceneObject, callOrder, stop);
		},
		std::addressof(handler));
}

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

template <typename T, typename Handler, typename>
bool SceneObject::ForEachComponent(Handler&& handler) noexcept {
	static_assert(std::is_base_of_v<ComponentImpl<T>, T>);
	
	return ForEachComponent(T::kType,
		+[](SceneObject sceneObject, Component* component, bool& stop, void* context) {
			std::invoke(std::forward<Handler>(*static_cast<Handler*>(context)), sceneObject, static_cast<T*>(component), stop);
		},
		std::addressof(handler));
}

template <typename T, typename Handler, typename>
bool SceneObject::ForEachComponentInParent(Handler&& handler) noexcept {
	static_assert(std::is_base_of_v<ComponentImpl<T>, T>);
	
	return ForEachComponentInParent(T::kType,
		+[](SceneObject sceneObject, Component* component, bool& stop, void* context) {
			std::invoke(std::forward<Handler>(*static_cast<Handler*>(context)), sceneObject, static_cast<T*>(component), stop);
		},
		std::addressof(handler));
}

template <typename T, typename Handler, typename>
bool SceneObject::ForEachComponentInChildren(Handler&& handler) noexcept {
	static_assert(std::is_base_of_v<ComponentImpl<T>, T>);
	
	return ForEachComponentInChildren(T::kType,
		+[](SceneObject sceneObject, Component* component, bool& stop, void* context) {
			std::invoke(std::forward<Handler>(*static_cast<Handler*>(context)), sceneObject, static_cast<T*>(component), stop);
		},
		std::addressof(handler));
}

