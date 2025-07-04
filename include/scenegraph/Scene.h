#pragma once

#include <scenegraph/memory/MonotonicAllocator.h>
#include <scenegraph/utils/Passkey.h>
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
	
private:
	std::unique_ptr<SceneNode> _root;
};

//---------------------------------------------------------------------------------------------------------------------

template <typename T, typename... Args>
std::unique_ptr<T> Scene::NewEntity(Passkey, Args&&... args) noexcept {
	static_assert(sizeof(T), "Type is not complete");
	static_assert(std::is_base_of_v<SceneEntity, T>);
	
	return std::unique_ptr<T>(std::construct_at(Allocate<T>(), std::forward<Args>(args)...));
}

//---------------------------------------------------------------------------------------------------------------------

template <typename T, typename>
T* SceneObject::AddComponent() noexcept {
	if (auto scene = GetScene()) {
		return static_cast<T*>(AddComponent(scene->NewEntity<T>(Scene::Passkey{})));
	}
	
	return nullptr;
}

//---------------------------------------------------------------------------------------------------------------------

template <typename T>
std::unique_ptr<Component> ComponentImpl<T>::Make(Scene* scene) noexcept {
	return scene->NewEntity<T>(Scene::Passkey{});
}
