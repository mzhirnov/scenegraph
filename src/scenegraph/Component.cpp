#include <scenegraph/Component.h>

void ComponentList::BroadcastMessage(ComponentMessage message, ComponentMessageParams& params) noexcept {
	for (auto it = begin(), e = end(); it != e; /**/) {
		if (auto& component = *it; !component.IsRemoved()) {
			component.SendMessage(message, params);
			++it;
		}
		else {
			it = Erase(it);
			component.SendMessage(ComponentMessage::Removed, params);
			delete std::addressof(component);
		}
	}
}
