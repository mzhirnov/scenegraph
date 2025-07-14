#pragma once

#include <scenegraph/SceneEntity.h>

class SceneString : public SceneEntity {
public:
	const char* c_str() const noexcept { return _str; }

private:
	char _str[1] {'\0'};
};
