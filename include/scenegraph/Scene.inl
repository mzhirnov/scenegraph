#pragma once

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
