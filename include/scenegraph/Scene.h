#pragma once

#include <scenegraph/memory/MonotonicAllocator.h>
#include <scenegraph/utils/StaticImpl.h>
#include <scenegraph/utils/NonCopyable.h>
#include <scenegraph/SceneObject.h>
#include <scenegraph/Component.h>
#include <scenegraph/SceneString.h>

#include <memory>
#include <string_view>
#include <type_traits>
#include <cstddef>

class SceneString;

using SceneAllocator = MonotonicAllocator<1 << 14>;

///
/// Scene is a container of scene objects hierarchy
///
class Scene : public SceneAllocator {
public:
	class Passkey : NonCopyableNonMovable {
		friend class Scene;
		friend class SceneObject;
		template <typename T> friend class ComponentImpl;
		
		constexpr explicit Passkey() = default;
	};
	
	Scene();
	
	~Scene();
	
	// Public field
	std::unique_ptr<Scene> nextScene;
	
	SceneObject GetRootObject() noexcept;
	
	SceneObject AddObject() noexcept;

	template <typename T, typename... Args>
	std::unique_ptr<T> NewEntity(Passkey, Args&&... args) noexcept;
	
	std::unique_ptr<SceneString> NewString(std::string_view str) noexcept;
	
	// void Handler(SceneObject sceneObject, bool& stop)
	template <typename Handler, typename = std::enable_if_t<std::is_invocable_v<Handler, SceneObject, bool&>>>
	bool ForEachObject(Handler&& handler) noexcept;
	
private:
	using EnumObjectsCallback = void(*)(SceneObject sceneObject, bool& stop, void* context);
	
	bool ForEachObject(EnumObjectsCallback callback, void* context) noexcept;
	
private:
	std::unique_ptr<SceneNode> _root;
};

#include "Scene.inl"
#include "SceneObject.inl"
#include "Component.inl"
