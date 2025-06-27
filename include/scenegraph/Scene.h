#pragma once

#include <scenegraph/memory/MonotonicAllocator.h>
#include <scenegraph/SceneObject.h>

#include <memory>
#include <type_traits>
#include <cstddef>

using SceneAllocator = MonotonicAllocator<1 << 14>;

///
/// Scene is a container of scene objects hierarchy
///
class Scene
	: public SceneAllocator
	, private SceneObject
{
public:
	Scene() = default;
	
	// Default behavior for new/delete here
	static void* operator new(std::size_t size) { return ::operator new(size); }
	static void operator delete(void* p, std::size_t size) { ::operator delete(p, size); }

private:
	template <typename T, typename... Args>
	T* NewEntity(Args&&... args) const noexcept {
		static_assert(sizeof(T), "Type is not complete");
		static_assert(std::is_base_of_v<SceneEntity, T>);
		
		auto entity = std::construct_at(Allocate<T>(), std::forward<Args>(args)...);
		return entity;
	}
};
