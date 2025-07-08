#pragma once

template <typename T>
std::unique_ptr<Component> ComponentImpl<T>::Make(Scene* scene) noexcept {
	return scene->NewEntity<T>(Scene::Passkey{});
}
