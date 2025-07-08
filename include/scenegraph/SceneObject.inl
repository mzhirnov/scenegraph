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
