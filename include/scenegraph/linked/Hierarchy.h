#pragma once

#include <type_traits>
#include <memory>
#include <functional>

enum class EnumDirection {
	FirstToLast,
	LastToFirst
};

enum class EnumCallOrder {
	PreOrder  = 1 << 0,
	PostOrder = 1 << 1
};

constexpr EnumCallOrder operator&(EnumCallOrder lhs, EnumCallOrder rhs) noexcept {
	return static_cast<EnumCallOrder>(
		static_cast<std::underlying_type_t<EnumCallOrder>>(lhs) &
		static_cast<std::underlying_type_t<EnumCallOrder>>(rhs));
}

constexpr EnumCallOrder operator|(EnumCallOrder lhs, EnumCallOrder rhs) noexcept {
	return static_cast<EnumCallOrder>(
		static_cast<std::underlying_type_t<EnumCallOrder>>(lhs) |
		static_cast<std::underlying_type_t<EnumCallOrder>>(rhs));
}

//	USAGE:
//
//	class Node : public Hierarchy<Node> {
//	};
//

///
///
///
template <typename NodeType>
class Hierarchy {
public:
	using EnumCallback = void(*)(EnumCallOrder callOrder, NodeType* currentNode, bool& stop, void* context);
	
	Hierarchy() noexcept = default;
	
	Hierarchy(const Hierarchy&) noexcept = default;
	Hierarchy& operator=(const Hierarchy&) noexcept { return *this; }
	
	Hierarchy(Hierarchy&&) noexcept;
	Hierarchy& operator=(Hierarchy&&) noexcept;
	
	~Hierarchy();

	// N a v i g a t i o n
	
	NodeType* GetParentNode() const noexcept;
	NodeType* GetNextSiblingNode() const noexcept;
	NodeType* GetPrevSiblingNode() const noexcept;
	NodeType* GetFirstChildNode() const noexcept;
	NodeType* GetLastChildNode() const noexcept;
	NodeType* GetChildNodeAt(int index) const noexcept;
	NodeType* GetRootNode() const noexcept;
	NodeType* GetLeastCommonAncestorNode(NodeType* node) const noexcept;
	
	bool ForEachChildNode(EnumDirection direction, EnumCallOrder callOrder, EnumCallback callback, void* context) const noexcept;
	
	// void Handler(EnumCallOrder callOrder, NodeType* currentNode, bool& stop)
	template <typename Handler, typename = std::enable_if_t<std::is_invocable_v<Handler, EnumCallOrder, NodeType*, bool&>>>
	bool ForEachChildNode(EnumDirection direction, EnumCallOrder callOrder, Handler&& handler) const noexcept;
	
	// M o d i f i c a t i o n
	
	NodeType* AppendChildNode(std::unique_ptr<NodeType> newChild) noexcept;
	NodeType* PrependChildNode(std::unique_ptr<NodeType> newChild) noexcept;
	NodeType* InsertChildNodeAt(std::unique_ptr<NodeType> newChild, int index) noexcept;
	NodeType* InsertNodeAfter(std::unique_ptr<NodeType> newSibling) noexcept;
	NodeType* InsertNodeBefore(std::unique_ptr<NodeType> newSibling) noexcept;
	std::unique_ptr<NodeType> ReplaceChildNode(NodeType* nodeToReplace, std::unique_ptr<NodeType> newNode) noexcept;
	std::unique_ptr<NodeType> RemoveChildNodeAt(int index) noexcept;
	std::unique_ptr<NodeType> RemoveFromParent() noexcept;
	void RemoveAllChildNodes() noexcept;
	
protected:
	static NodeType* InsertNodeAfter(NodeType* node, std::unique_ptr<NodeType> newChild) noexcept;
	static NodeType* InsertNodeBefore(NodeType* node, std::unique_ptr<NodeType> newChild) noexcept;
	static std::unique_ptr<NodeType> RemoveNode(NodeType* nodeToRemove) noexcept;

private:
	NodeType* _parentNode = nullptr;
	NodeType* _prevSiblingNode = static_cast<NodeType*>(this);
	std::unique_ptr<NodeType> _nextSiblingNode;
	std::unique_ptr<NodeType> _firstChildNode;
};

//---------------------------------------------------------------------------------------------------------------------

template <typename NodeType>
Hierarchy<NodeType>::Hierarchy(Hierarchy&& rhs) noexcept
	: _parentNode(rhs._parentNode)
	, _prevSiblingNode(
		rhs._prevSiblingNode == static_cast<NodeType*>(&rhs) ?
			static_cast<NodeType*>(this) :
			rhs._prevSiblingNode)
	, _nextSiblingNode(std::move(rhs._nextSiblingNode))
	, _firstChildNode(std::move(rhs._firstChildNode))
{
	for (auto child = GetFirstChildNode(); child; child = child->GetNextSiblingNode()) {
		child->_parentNode = static_cast<NodeType*>(this);
	}
}

template <typename NodeType>
Hierarchy<NodeType>& Hierarchy<NodeType>::operator=(Hierarchy&& rhs) noexcept {
	if (this != &rhs) {
		_parentNode = rhs._parentNode;
		_prevSiblingNode =
			rhs._prevSiblingNode == static_cast<NodeType*>(&rhs) ?
				static_cast<NodeType*>(this) :
				rhs._prevSiblingNode;
		_nextSiblingNode = std::move(rhs._nextSiblingNode);
		_firstChildNode = std::move(rhs._firstChildNode);
		
		for (auto child = GetFirstChildNode(); child; child = child->GetNextSiblingNode()) {
			child->_parentNode = static_cast<NodeType*>(this);
		}
	}
	
	return *this;
}

template <typename NodeType>
Hierarchy<NodeType>::~Hierarchy() {
	ForEachChildNode(EnumDirection::LastToFirst, EnumCallOrder::PostOrder,
		[](EnumCallOrder, NodeType* currentNode, bool&, void*) {
			currentNode->RemoveAllChildNodes();
		}, nullptr);

	RemoveAllChildNodes();
}

template <typename NodeType>
NodeType* Hierarchy<NodeType>::GetParentNode() const noexcept {
	return _parentNode;
}

template <typename NodeType>
NodeType* Hierarchy<NodeType>::GetPrevSiblingNode() const noexcept {
	return _prevSiblingNode->_nextSiblingNode ? _prevSiblingNode : nullptr;
}

template <typename NodeType>
NodeType* Hierarchy<NodeType>::GetNextSiblingNode() const noexcept {
	return _nextSiblingNode.get();
}

template <typename NodeType>
NodeType* Hierarchy<NodeType>::GetFirstChildNode() const noexcept {
	return _firstChildNode.get();
}

template <typename NodeType>
NodeType* Hierarchy<NodeType>::GetLastChildNode() const noexcept {
	return _firstChildNode ? _firstChildNode->_prevSiblingNode : nullptr;
}

template <typename NodeType>
NodeType* Hierarchy<NodeType>::GetChildNodeAt(int index) const noexcept {
	if (index >= 0) {
		auto child = GetFirstChildNode();
		while (child && index-- > 0) {
			child = child->GetNextSiblingNode();
		}
		return child;
	}
	else {
		auto child = GetLastChildNode();
		while (child && ++index < 0) {
			child = child->GetPrevSiblingNode();
		}
		return child;
	}
}

template <typename NodeType>
NodeType* Hierarchy<NodeType>::GetRootNode() const noexcept {
	if (auto currentNode = GetParentNode()) {
		while (auto parent = currentNode->GetParentNode()) {
			currentNode = parent;
		}
		return currentNode;
	}
	return nullptr;
}

template <typename NodeType>
NodeType* Hierarchy<NodeType>::GetLeastCommonAncestorNode(NodeType* node) const noexcept {
	assert(node != nullptr);
	
	if (!node) {
		return nullptr;
	}
	
	auto thisAncestor = static_cast<const NodeType*>(this);
	auto nodeAncestor = node;
	
	while (thisAncestor && nodeAncestor) {
		if (thisAncestor == nodeAncestor) {
			return nodeAncestor;
		}
		thisAncestor = thisAncestor->_parentNode;
		nodeAncestor = nodeAncestor->_parentNode;
	}
	
	if (!thisAncestor && !nodeAncestor) {
		return nullptr;
	}

	auto farthestAncestor = thisAncestor ? thisAncestor : nodeAncestor;
	auto reentryAncestor1 = thisAncestor ? static_cast<const NodeType*>(this) : node;
	auto reentryAncestor2 = thisAncestor ? node : static_cast<const NodeType*>(this);
	
	while (farthestAncestor) {
		farthestAncestor = farthestAncestor->_parentNode;
		reentryAncestor1 = reentryAncestor1->_parentNode;
	}
	
	return reentryAncestor1->GetLeastCommonAncestorNode(const_cast<NodeType*>(reentryAncestor2));
}

template <typename NodeType>
bool Hierarchy<NodeType>::ForEachChildNode(EnumDirection direction, EnumCallOrder callOrder, EnumCallback callback, void* context) const noexcept {
	assert(callback != nullptr);
	
	if (!callback) {
		return false;
	}
	
	const auto doCallPreOrder = (callOrder & EnumCallOrder::PreOrder) == EnumCallOrder::PreOrder;
	const auto doCallPostOrder = (callOrder & EnumCallOrder::PostOrder) == EnumCallOrder::PostOrder;
	
	auto GetFirstChildNode = (direction == EnumDirection::FirstToLast) ?
		&Hierarchy::GetFirstChildNode :
		&Hierarchy::GetLastChildNode;
	
	auto GetNextSiblingNode = (direction == EnumDirection::FirstToLast) ?
		&Hierarchy::GetNextSiblingNode :
		&Hierarchy::GetPrevSiblingNode;
	
	auto stop = false;
	auto currentNode = (this->*GetFirstChildNode)();
	
	while (currentNode && currentNode != static_cast<const NodeType*>(this) && !stop) {
		// PreOrder
		if (doCallPreOrder) {
			callback(EnumCallOrder::PreOrder, currentNode, stop, context);
			if (stop) {
				break;
			}
		}

		if (auto firstChildNode = (currentNode->*GetFirstChildNode)()) {
			currentNode = firstChildNode;
		}
		else if (auto nextSiblingNode = (currentNode->*GetNextSiblingNode)()) {
			// PostOrder
			if (doCallPostOrder) {
				callback(EnumCallOrder::PostOrder, currentNode, stop, context);
				if (stop) {
					break;
				}
			}
			
			currentNode = nextSiblingNode;
		}
		else {
			// PostOrder
			if (doCallPostOrder) {
				callback(EnumCallOrder::PostOrder, currentNode, stop, context);
				if (stop) {
					break;
				}
			}
			
			while (!(currentNode->*GetNextSiblingNode)() && currentNode != static_cast<const NodeType*>(this)) {
				currentNode = currentNode->GetParentNode();
				// PostOrder
				if (doCallPostOrder) {
					if (currentNode != static_cast<const NodeType*>(this)) {
						callback(EnumCallOrder::PostOrder, currentNode, stop, context);
						if (stop) {
							break;
						}
					}
				}
			}

			if (currentNode != static_cast<const NodeType*>(this)) {
				currentNode = (currentNode->*GetNextSiblingNode)();
			}
		}
	}
	
	return stop;
}

template <typename NodeType>
template <typename Handler, typename>
bool Hierarchy<NodeType>::ForEachChildNode(EnumDirection direction, EnumCallOrder callOrder, Handler&& handler) const noexcept {
	return ForEachChildNode(direction, callOrder,
		+[](EnumCallOrder callOrder, NodeType* currentNode, bool& stop, void* context) {
			std::invoke(std::forward<Handler>(*static_cast<Handler*>(context)), callOrder, currentNode, stop);
		},
		std::addressof(handler));
}

template <typename NodeType>
NodeType* Hierarchy<NodeType>::AppendChildNode(std::unique_ptr<NodeType> child) noexcept {
	assert(child != nullptr);
	
	if (!child) {
		return nullptr;
	}
	
	child->_parentNode = static_cast<NodeType*>(this);
	
	auto insertedNode = child.get();
	
	if (auto head = _firstChildNode.get()) {
		auto tail = std::exchange(head->_prevSiblingNode, child.get());
		child->_prevSiblingNode = tail;
		tail->_nextSiblingNode = std::move(child);
	}
	else {
		_firstChildNode = std::move(child);
	}
	
	return insertedNode;
}

template <typename NodeType>
NodeType* Hierarchy<NodeType>::PrependChildNode(std::unique_ptr<NodeType> child) noexcept {
	assert(child != nullptr);
	
	if (!child) {
		return nullptr;
	}
	
	child->_parentNode = static_cast<NodeType*>(this);
	
	auto insertedNode = child.get();

	if (_firstChildNode) {
		child->_prevSiblingNode = std::exchange(_firstChildNode->_prevSiblingNode, child.get());
		child->_nextSiblingNode = std::move(_firstChildNode);
	}
	else {
		child->_prevSiblingNode = child.get();
		child->_nextSiblingNode = nullptr;
	}

	_firstChildNode = std::move(child);
	
	return insertedNode;
}

template <typename NodeType>
NodeType* Hierarchy<NodeType>::InsertChildNodeAt(std::unique_ptr<NodeType> newChild, int index) noexcept {
	auto node = GetChildNodeAt(index);
	if (index >= 0) {
		return node ? InsertNodeBefore(node, std::move(newChild)) : AppendChildNode(std::move(newChild));
	}
	else {
		return node ? InsertNodeAfter(node, std::move(newChild)) : PrependChildNode(std::move(newChild));
	}
}

template <typename NodeType>
NodeType* Hierarchy<NodeType>::InsertNodeAfter(std::unique_ptr<NodeType> newSibling) noexcept {
	return InsertNodeAfter(static_cast<NodeType*>(this), std::move(newSibling));
}

template <typename NodeType>
NodeType* Hierarchy<NodeType>::InsertNodeBefore(std::unique_ptr<NodeType> newSibling) noexcept {
	return InsertNodeBefore(static_cast<NodeType*>(this), std::move(newSibling));
}

template <typename NodeType>
std::unique_ptr<NodeType> Hierarchy<NodeType>::ReplaceChildNode(NodeType* nodeToReplace, std::unique_ptr<NodeType> newNode) noexcept {
	assert(nodeToReplace != nullptr);
	assert(newNode != nullptr);
	
	if (!nodeToReplace || !newNode) {
		return {};
	}
	
	assert(nodeToReplace->_parentNode == static_cast<NodeType*>(this));
	
	if (nodeToReplace->_parentNode != static_cast<NodeType*>(this)) {
		return {};
	}
	
	assert(newNode->_parentNode == nullptr);
	
	if (newNode->_parentNode != nullptr) {
		newNode = RemoveNode(newNode.get());
	}
	
	if (nodeToReplace->_nextSiblingNode) {
		InsertNodeBefore(nodeToReplace->_nextSiblingNode.get(), std::move(newNode));
	}
	else {
		AppendChildNode(std::move(newNode));
	}
	
	return RemoveNode(nodeToReplace);
}

template <typename NodeType>
std::unique_ptr<NodeType> Hierarchy<NodeType>::RemoveChildNodeAt(int index) noexcept {
	if (auto child = GetChildNodeAt(index)) {
		return RemoveNode(child);
	}
	return {};
}

template <typename NodeType>
std::unique_ptr<NodeType> Hierarchy<NodeType>::RemoveFromParent() noexcept {
	return RemoveNode(static_cast<NodeType*>(this));
}

template <typename NodeType>
void Hierarchy<NodeType>::RemoveAllChildNodes() noexcept {
	for (auto child = GetLastChildNode(); child; child = child->GetPrevSiblingNode()) {
		RemoveNode(child);
	}
	_firstChildNode = nullptr;
}

template <typename NodeType>
NodeType* Hierarchy<NodeType>::InsertNodeAfter(NodeType* node, std::unique_ptr<NodeType> newChild) noexcept {
	assert(node != nullptr);
	assert(newChild != nullptr);
	
	if (!node || !newChild) {
		return nullptr;
	}
	
	auto parent = node->_parentNode;
	
	assert(parent != nullptr);
	
	if (!parent) {
		return nullptr;
	}

	newChild->_parentNode = parent;
	newChild->_prevSiblingNode = node;
	
	auto insertedNode = newChild.get();

	if (node->_nextSiblingNode) {
		node->_nextSiblingNode->_prevSiblingNode = insertedNode;
	}
	else {
		parent->_firstChildNode->_prevSiblingNode = insertedNode;
	}

	newChild->_nextSiblingNode = std::move(node->_nextSiblingNode);
	node->_nextSiblingNode = std::move(newChild);
	
	return insertedNode;
}

template <typename NodeType>
NodeType* Hierarchy<NodeType>::InsertNodeBefore(NodeType* node, std::unique_ptr<NodeType> newChild) noexcept {
	assert(node != nullptr);
	assert(newChild != nullptr);
	
	if (!node || !newChild) {
		return nullptr;
	}
	
	auto parent = node->_parentNode;
	
	assert(parent != nullptr);
	
	if (!parent) {
		return nullptr;
	}

	newChild->_parentNode = parent;
	newChild->_prevSiblingNode = node->_prevSiblingNode;
	
	auto insertedNode = newChild.get();

	if (node->_prevSiblingNode->_nextSiblingNode) {
		newChild->_nextSiblingNode = std::move(node->_prevSiblingNode->_nextSiblingNode);
		node->_prevSiblingNode->_nextSiblingNode = std::move(newChild);
	}
	else {
		newChild->_nextSiblingNode = std::move(parent->_firstChildNode);
		parent->_firstChildNode = std::move(newChild);
	}
	
	node->_prevSiblingNode = insertedNode;
	
	return insertedNode;
}

template <typename NodeType>
std::unique_ptr<NodeType> Hierarchy<NodeType>::RemoveNode(NodeType* nodeToRemove) noexcept {
	assert(nodeToRemove != nullptr);
	
	if (!nodeToRemove) {
		return {};
	}
	
	auto parent = nodeToRemove->_parentNode;
	
	assert(parent != nullptr);
	
	if (!parent) {
		return {};
	}
	
	if (nodeToRemove->_nextSiblingNode) {
		nodeToRemove->_nextSiblingNode->_prevSiblingNode = nodeToRemove->_prevSiblingNode;
	}
	else {
		parent->_firstChildNode->_prevSiblingNode = nodeToRemove->_prevSiblingNode;
	}
	
	std::unique_ptr<NodeType> detachedNode;

	if (nodeToRemove->_prevSiblingNode->_nextSiblingNode) {
		detachedNode = std::move(nodeToRemove->_prevSiblingNode->_nextSiblingNode);
		nodeToRemove->_prevSiblingNode->_nextSiblingNode = std::move(nodeToRemove->_nextSiblingNode);
	}
	else {
		detachedNode = std::move(parent->_firstChildNode);
		parent->_firstChildNode = std::move(nodeToRemove->_nextSiblingNode);
	}

	detachedNode->_parentNode = nullptr;
	detachedNode->_prevSiblingNode = detachedNode.get();
	detachedNode->_nextSiblingNode = nullptr;
	
	return detachedNode;
}
