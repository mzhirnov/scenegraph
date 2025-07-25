#include <scenegraph/SceneString.h>
#include <scenegraph/Scene.h>

std::string_view SceneString::ToStringView() const noexcept {
	return {_str};
}
