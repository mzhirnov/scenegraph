#include <scenegraph/SceneString.h>
#include <scenegraph/Scene.h>

std::string_view SceneString::ToStringView() const noexcept {
	auto size = SceneAllocator::GetSize(_str);
	return {_str, size};
}
