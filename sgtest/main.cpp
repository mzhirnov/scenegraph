#include <gtest/gtest.h>
#include <scenegraph/ForwardList.h>
#include <scenegraph/Hierarchy.h>
#include <scenegraph/PoolAllocator.h>
#include <scenegraph/MonotonicAllocator.h>
#include <string>

class node : public ForwardListNode<> {
public:
	explicit node(std::string_view name) : name{name} {}

public:
	std::string name;
};

class object : public Hierarchy<object> {
public:
	explicit object(std::string_view name) : name{name} {}

public:
	std::string name;
};

template <typename List>
void TestPushFront() {
	List list;

	node n1{"n1"};
	node n2{"n2"};
	node n3{"n3"};

	list.PushFront(n1);
	list.PushFront(n2);
	list.PushFront(n3);

	ASSERT_TRUE(!list.Empty());

	auto it = list.begin();
	ASSERT_EQ(it->name, "n3");
	++it;
	ASSERT_EQ(it->name, "n2");
	++it;
	ASSERT_EQ(it->name, "n1");
	++it;
	ASSERT_EQ(it, list.end());
}

template <typename List>
void TestPushBack() {
	List list;

	node n1{"n1"};
	node n2{"n2"};
	node n3{"n3"};

	list.PushBack(n1);
	list.PushBack(n2);
	list.PushBack(n3);

	ASSERT_TRUE(!list.Empty());
	
	auto it = list.begin();
	ASSERT_EQ(it->name, "n1");
	++it;
	ASSERT_EQ(it->name, "n2");
	++it;
	ASSERT_EQ(it->name, "n3");
	++it;
	ASSERT_EQ(it, list.end());
}

template <typename List>
void TestReverse() {
	List list;

	node n1{"n1"};
	node n2{"n2"};
	node n3{"n3"};

	list.PushFront(n1);
	list.PushFront(n2);
	list.PushFront(n3);

	ASSERT_TRUE(!list.Empty());

	list.Reverse();

	auto it = list.begin();
	ASSERT_EQ(it->name, "n1");
	++it;
	ASSERT_EQ(it->name, "n2");
	++it;
	ASSERT_EQ(it->name, "n3");
	++it;
	ASSERT_EQ(it, list.end());
}

template <typename List>
void TestRotate() {
	List list;

	node n1{"n1"};
	node n2{"n2"};
	node n3{"n3"};

	list.PushFront(n1);
	list.PushFront(n2);
	list.PushFront(n3);

	ASSERT_TRUE(!list.Empty());

	ASSERT_EQ(list.Front().name, "n3");
	list.Rotate();
	ASSERT_EQ(list.Front().name, "n2");
	list.Rotate();
	ASSERT_EQ(list.Front().name, "n1");
	list.Rotate();
	ASSERT_EQ(list.Front().name, "n3");
}

template <typename List>
void TestErase() {
	List list;

	node n1{"n1"};
	node n2{"n2"};
	node n3{"n3"};

	list.PushFront(n1);
	list.PushFront(n2);
	list.PushFront(n3);

	list.Erase(list.begin());
	{
		auto it = list.begin();
		ASSERT_EQ(it->name, "n2");
		++it;
		ASSERT_EQ(it->name, "n1");
		++it;
		ASSERT_EQ(it, list.end());
	}

	list.Erase(list.begin());
	{
		auto it = list.begin();
		ASSERT_EQ(it->name, "n1");
		++it;
		ASSERT_EQ(it, list.end());
	}

	list.Erase(list.begin());
	{
		auto it = list.begin();
		ASSERT_EQ(it, list.end());
	}
}

template <typename List>
void TestErase2() {
	List list;

	node n1{"n1"};
	node n2{"n2"};
	node n3{"n3"};

	list.PushFront(n1);
	list.PushFront(n2);
	list.PushFront(n3);

	list.Erase(std::next(list.begin()));
	{
		auto it = list.begin();
		ASSERT_EQ(it->name, "n3");
		++it;
		ASSERT_EQ(it->name, "n1");
		++it;
		ASSERT_EQ(it, list.end());
	}

	list.Erase(std::next(list.begin()));
	{
		auto it = list.begin();
		ASSERT_EQ(it->name, "n3");
		++it;
		ASSERT_EQ(it, list.end());
	}

	list.Erase(list.begin());
	{
		auto it = list.begin();
		ASSERT_EQ(it, list.end());
	}
}

template <typename List>
void TestErase3() {
	List list;

	node n1{"n1"};
	node n2{"n2"};
	node n3{"n3"};

	list.PushFront(n1);
	list.PushFront(n2);
	list.PushFront(n3);

	list.Erase(std::next(list.begin(), 2));
	{
		auto it = list.begin();
		ASSERT_EQ(it->name, "n3");
		++it;
		ASSERT_EQ(it->name, "n2");
		++it;
		ASSERT_EQ(it, list.end());
	}

	list.Erase(std::next(list.begin()));
	{
		auto it = list.begin();
		ASSERT_EQ(it->name, "n3");
		++it;
		ASSERT_EQ(it, list.end());
	}

	list.Erase(list.begin());
	{
		auto it = list.begin();
		ASSERT_EQ(it, list.end());
	}

	list.Erase(list.end());
	{
		auto it = list.begin();
		ASSERT_EQ(it, list.end());
	}
}

using list = ForwardList<node>;
using circular_list = CircularForwardList<node>;

//---------------------------------------------------------------------------------------------------------------------

TEST(List, PushFront) { TestPushFront<list>(); }
TEST(List, PushBack) { TestPushBack<list>(); }
TEST(List, Reverse) { TestReverse<list>(); }
TEST(List, Rotate) { TestRotate<list>(); }
TEST(List, Erase) { TestErase<list>(); }
TEST(List, Erase2) { TestErase2<list>(); }
TEST(List, Erase3) { TestErase3<list>(); }

//---------------------------------------------------------------------------------------------------------------------

TEST(CircularList, PushFront) { TestPushFront<circular_list>(); }
TEST(CircularList, PushBack) { TestPushBack<circular_list>(); }
TEST(CircularList, Reverse) { TestReverse<circular_list>(); }
TEST(CircularList, Rotate) { TestRotate<circular_list>(); }
TEST(CircularList, Erase) { TestErase<circular_list>(); }
TEST(CircularList, Erase2) { TestErase2<circular_list>(); }
TEST(CircularList, Erase3) { TestErase3<circular_list>(); }

//---------------------------------------------------------------------------------------------------------------------

TEST(Hierarchy, Create) {
	object root{"root"};
	
	ASSERT_EQ(root.GetParentNode(), nullptr);
	ASSERT_EQ(root.GetFirstChildNode(), nullptr);
	ASSERT_EQ(root.GetPrevSiblingNode(), nullptr);
	ASSERT_EQ(root.GetNextSiblingNode(), nullptr);
}

TEST(PoolAllocator, Allocate) {
	PoolAllocator<int, 2> allocator;
	
	auto p = allocator.Allocate();
	void* addr1 = p;
	void* addr2;
	void* addr3;
	
	{
		addr2 = allocator.Allocate();
		EXPECT_NE(addr1, addr2);
		allocator.Deallocate(addr2);
	}
	{
		addr3 = allocator.Allocate();
		EXPECT_NE(addr1, addr3);
		allocator.Deallocate(addr3);
	}
	
	EXPECT_EQ(addr2, addr3);
	
	allocator.Deallocate(p);
	
	auto p1 = allocator.Allocate();
	auto p2 = allocator.Allocate();
	auto p3 = allocator.Allocate();
	
	EXPECT_EQ(addr1, p1);
	EXPECT_EQ(addr2, p2);
	EXPECT_NE(addr3, p3);
	
	allocator.Deallocate(p1);
	allocator.Deallocate(p2);
	allocator.Deallocate(p3);
}

TEST(MonotonicAllocator, GetAllocator) {
	using Allocator = MonotonicAllocator<48>;
	
	Allocator allocator;
	
	auto* p1 = allocator.Allocate<int>();
	auto* p2 = allocator.Allocate<int>();
	
	auto a1 = Allocator::GetAllocator(p1);
	auto a2 = Allocator::GetAllocator(p2);
	
	EXPECT_EQ(a1, std::addressof(allocator));
	EXPECT_EQ(a2, std::addressof(allocator));
	
	allocator.Deallocate(p1);
	allocator.Deallocate(p2);
	
	//----------------------------------------------------------------------------
	
	auto* p3 = allocator.Allocate<int>();
	auto* p4 = allocator.Allocate<int>();
	
	auto a3 = Allocator::GetAllocator(p3);
	auto a4 = Allocator::GetAllocator(p4);
	
	EXPECT_EQ(a3, std::addressof(allocator));
	EXPECT_EQ(a4, std::addressof(allocator));
	
	allocator.Deallocate(p3);
	allocator.Deallocate(p4);
}
