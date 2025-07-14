#pragma once

#include <cstddef>

///
/// Scene entity belongs to a scene and is managed by it's memory manager
///
class SceneEntity {
public:
	class Scene* GetScene() noexcept;
	
	static void* operator new(std::size_t, ...) = delete;
	static void* operator new[](std::size_t, ...) = delete;
	
	static void operator delete(void* p) noexcept; // Only implemented
	static void operator delete[](void*, ...) = delete;
};
