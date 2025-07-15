#pragma once

template <typename T, typename... Args>
std::unique_ptr<T> Scene::NewEntity(Passkey, Args&&... args) noexcept {
	static_assert(sizeof(T) > 0, "Type is not complete");
	static_assert(std::is_base_of_v<SceneEntity, T>);
	
	return std::unique_ptr<T>(std::construct_at(Allocate<T>(), std::forward<Args>(args)...));
}

template <typename Handler, typename>
bool Scene::ForEachRootObject(Handler&& handler) noexcept {
	return ForEachRootObject(
		+[](SceneObject sceneObject, bool& stop, void* context) {
			std::invoke(std::forward<Handler>(*static_cast<Handler*>(context)), sceneObject, stop);
		},
		std::addressof(handler));
}

template <typename Handler, typename>
bool Scene::WalkObjects(EnumDirection direction, EnumCallOrder callOrder, Handler&& handler) noexcept {
	return SceneObject{_root.GetPtr()}.WalkChildren(direction, callOrder, std::forward<Handler>(handler));
}
