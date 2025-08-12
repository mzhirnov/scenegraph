#pragma once

#include <scenegraph/Component.h>
#include <scenegraph/SceneObject.h>
#include <scenegraph/geometry/Transform2D.h>
#include <scenegraph/geometry/Matrix32.h>

class Transform2DComponent final : public ComponentImpl<Transform2DComponent> {
public:
	DEFINE_COMPONENT_TYPE(Transform2DComponent)
	
	Transform2D localTransform = Transform2DMakeIdentity();
	
	const Matrix32& GetWorldTransform() const noexcept { return _worldTransform; }

private:
	friend Super;
	
	void Apply(SceneObject sceneObject) noexcept {
		CalculateWorldTransform(sceneObject);
	}
	
private:
	void CalculateWorldTransform(SceneObject sceneObject) noexcept {
		_worldTransform = Matrix32MakeWithTransform2D(localTransform);
		
#if 0
		sceneObject.ForEachComponentInParent<Transform2DComponent>(
			[this](SceneObject, Transform2DComponent* parentTransform, bool& stop) {
				_worldTransform = parentTransform->_worldTransform * _worldTransform;
				stop = true;
			});
#else
		auto parent = sceneObject;
		while ((parent = parent.Parent())) {
			if (auto parentTransform = parent.FindComponent<Transform2DComponent>()) {
				_worldTransform = parentTransform->_worldTransform * _worldTransform;
				break;
			}
		}
#endif
	}
	
private:
	Matrix32 _worldTransform = Matrix32MakeIdentity();
};
