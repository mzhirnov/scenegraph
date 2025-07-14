#pragma once

#include <scenegraph/SceneEntity.h>
#include <scenegraph/utils/NonCopyable.h>

#include <string_view>

///
/// String within scene's memory manager
///
class SceneString : public NonCopyableNonMovable, public SceneEntity {
public:
	const char* c_str() const noexcept { return _str; }
	
	std::string_view ToStringView() const noexcept;

private:
	char _str[1] {'\0'};
};
