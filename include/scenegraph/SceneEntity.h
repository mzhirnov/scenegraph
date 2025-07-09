#pragma once

#include <cstddef>

class Scene;

///
/// Scene entity belongs to a scene and is managed by it's memory manager
///
class SceneEntity {
public:
	Scene* GetScene() noexcept;
	
	static void* operator new(std::size_t, ...) = delete;
	static void* operator new[](std::size_t, ...) = delete;
	
	static void operator delete(void* p) noexcept;
	static void operator delete[](void*, ...) = delete;
};
