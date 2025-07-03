#include "SceneNode.h"

SceneNode::~SceneNode() {
	for (auto& c : _components) {
		delete std::addressof(c);
	}
}
