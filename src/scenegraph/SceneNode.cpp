#include "SceneNode.h"

SceneNode::~SceneNode() {
	for (auto& c : components) {
		delete std::addressof(c);
	}
}
