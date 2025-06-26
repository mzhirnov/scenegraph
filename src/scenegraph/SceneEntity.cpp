#include <scenegraph/SceneEntity.h>
#include <scenegraph/Scene.h>

Scene* SceneEntity::GetScene() noexcept {
	return static_cast<Scene*>(SceneAllocator::GetAllocator(this));
}

void SceneEntity::operator delete(void* p, std::size_t size) noexcept {
	auto allocator = SceneAllocator::GetAllocator(p);
	allocator->Deallocate(p, size);
}
