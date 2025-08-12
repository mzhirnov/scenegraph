#pragma once

#include <scenegraph/Component.h>
#include <scenegraph/geometry/Transform.h>

class TransformComponent : public ComponentImpl<TransformComponent> {
public:
	DEFINE_COMPONENT_TYPE(TransformComponent)

private:
	friend Super;
};
