#include <gtest/gtest.h>

#include <scenegraph/linked/ForwardList.h>
#include <scenegraph/linked/Hierarchy.h>
#include <scenegraph/memory/PoolAllocator.h>
#include <scenegraph/memory/MonotonicAllocator.h>
#include <scenegraph/utils/ScopeGuard.h>

#include <string>

class Node : public ForwardListNode<Node> {
public:
	explicit Node(std::string_view name) : name{name} {}
	
	bool operator<(const Node& other) const noexcept { return name < other.name; }
	bool operator>(const Node& other) const noexcept { return name > other.name; }

public:
	std::string name;
};

class Object : public Hierarchy<Object> {
public:
	explicit Object(std::string_view name) : name{name} {}

public:
	std::string name;
};

template <typename List>
void TestPushFront() {
	List list;

	Node n1{"n1"};
	Node n2{"n2"};
	Node n3{"n3"};

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

	Node n1{"n1"};
	Node n2{"n2"};
	Node n3{"n3"};

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

	Node n1{"n1"};
	Node n2{"n2"};
	Node n3{"n3"};

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

	Node n1{"n1"};
	Node n2{"n2"};
	Node n3{"n3"};

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

	Node n1{"n1"};
	Node n2{"n2"};
	Node n3{"n3"};

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

	Node n1{"n1"};
	Node n2{"n2"};
	Node n3{"n3"};

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

	Node n1{"n1"};
	Node n2{"n2"};
	Node n3{"n3"};

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
}

template <typename List>
void TestMerge() {
	List listA, listB;
	
	Node a1{"1"};
	Node a2{"2"};
	Node a3{"3"};
	
	listA.PushFront(a3);
	listA.PushFront(a2);
	listA.PushFront(a1);
	
	Node b1{"0"};
	Node b2{"1"};
	Node b3{"3"};
	Node b4{"9"};
	
	listB.PushFront(b4);
	listB.PushFront(b3);
	listB.PushFront(b2);
	listB.PushFront(b1);
	
	listA.Merge(listB);
	
	EXPECT_FALSE(listA.Empty());
	EXPECT_TRUE(listB.Empty());
	
	auto it = listA.begin();
	ASSERT_TRUE(it != listA.end());
	EXPECT_EQ(it->name, "0");
	++it;
	ASSERT_TRUE(it != listA.end());
	EXPECT_EQ(it->name, "1");
	++it;
	ASSERT_TRUE(it != listA.end());
	EXPECT_EQ(it->name, "1");
	++it;
	ASSERT_TRUE(it != listA.end());
	EXPECT_EQ(it->name, "2");
	++it;
	ASSERT_TRUE(it != listA.end());
	EXPECT_EQ(it->name, "3");
	++it;
	ASSERT_TRUE(it != listA.end());
	EXPECT_EQ(it->name, "3");
	++it;
	ASSERT_TRUE(it != listA.end());
	EXPECT_EQ(it->name, "9");
	++it;
	ASSERT_TRUE(it == listA.end());
	
	listA.Rotate(6);
	EXPECT_EQ(listA.Front().name, "9");
	EXPECT_EQ(listA.Back().name, "3");
}

template <typename List>
void TestAppend() {
	List listA, listB;
	
	Node a1{"1"};
	Node a2{"2"};
	Node a3{"3"};
	
	listA.PushFront(a3);
	listA.PushFront(a2);
	listA.PushFront(a1);
	
	Node b1{"0"};
	Node b2{"1"};
	Node b3{"3"};
	Node b4{"9"};
	
	listB.PushFront(b4);
	listB.PushFront(b3);
	listB.PushFront(b2);
	listB.PushFront(b1);
	
	listA.AppendList(listB);
	
	EXPECT_FALSE(listA.Empty());
	EXPECT_TRUE(listB.Empty());
	
	auto it = listA.begin();
	ASSERT_TRUE(it != listA.end());
	EXPECT_EQ(it->name, "1");
	++it;
	ASSERT_TRUE(it != listA.end());
	EXPECT_EQ(it->name, "2");
	++it;
	ASSERT_TRUE(it != listA.end());
	EXPECT_EQ(it->name, "3");
	++it;
	ASSERT_TRUE(it != listA.end());
	EXPECT_EQ(it->name, "0");
	++it;
	ASSERT_TRUE(it != listA.end());
	EXPECT_EQ(it->name, "1");
	++it;
	ASSERT_TRUE(it != listA.end());
	EXPECT_EQ(it->name, "3");
	++it;
	ASSERT_TRUE(it != listA.end());
	EXPECT_EQ(it->name, "9");
	++it;
	ASSERT_TRUE(it == listA.end());
	
	listA.Rotate(6);
	EXPECT_EQ(listA.Front().name, "9");
}

template <typename List>
void TestSort() {
	List list;

	Node n1{"4"};
	Node n2{"1"};
	Node n3{"3"};
	Node n4{"2"};
	Node n5{"5"};

	list.PushFront(n1);
	list.PushFront(n2);
	list.PushFront(n3);
	list.PushFront(n4);
	list.PushFront(n5);
	
	list.Sort();
	
	EXPECT_FALSE(list.Empty());
	
	auto it = list.begin();
	ASSERT_TRUE(it != list.end());
	EXPECT_EQ(it->name, "1");
	++it;
	ASSERT_TRUE(it != list.end());
	EXPECT_EQ(it->name, "2");
	++it;
	ASSERT_TRUE(it != list.end());
	EXPECT_EQ(it->name, "3");
	++it;
	ASSERT_TRUE(it != list.end());
	EXPECT_EQ(it->name, "4");
	++it;
	ASSERT_TRUE(it != list.end());
	EXPECT_EQ(it->name, "5");
	++it;
	ASSERT_TRUE(it == list.end());
	
	list.Rotate(3);
	EXPECT_EQ(list.Front().name, "4");
	
	list.Sort(std::greater<typename List::value_type>{});
	
	EXPECT_FALSE(list.Empty());
	
	it = list.begin();
	ASSERT_TRUE(it != list.end());
	EXPECT_EQ(it->name, "5");
	++it;
	ASSERT_TRUE(it != list.end());
	EXPECT_EQ(it->name, "4");
	++it;
	ASSERT_TRUE(it != list.end());
	EXPECT_EQ(it->name, "3");
	++it;
	ASSERT_TRUE(it != list.end());
	EXPECT_EQ(it->name, "2");
	++it;
	ASSERT_TRUE(it != list.end());
	EXPECT_EQ(it->name, "1");
	++it;
	ASSERT_TRUE(it == list.end());
	
	list.Rotate(3);
	EXPECT_EQ(list.Front().name, "2");
}

using List = ForwardList<Node>;
using CircularList = CircularForwardList<Node>;

//---------------------------------------------------------------------------------------------------------------------

TEST(List, PushFront) { TestPushFront<List>(); }
TEST(List, PushBack) { TestPushBack<List>(); }
TEST(List, Reverse) { TestReverse<List>(); }
TEST(List, Rotate) { TestRotate<List>(); }
TEST(List, Erase) { TestErase<List>(); }
TEST(List, Erase2) { TestErase2<List>(); }
TEST(List, Erase3) { TestErase3<List>(); }
TEST(List, Append) { TestAppend<List>(); }
TEST(List, Merge) { TestMerge<List>(); }
TEST(List, Sort) { TestSort<List>(); }

//---------------------------------------------------------------------------------------------------------------------

TEST(CircularList, PushFront) { TestPushFront<CircularList>(); }
TEST(CircularList, PushBack) { TestPushBack<CircularList>(); }
TEST(CircularList, Reverse) { TestReverse<CircularList>(); }
TEST(CircularList, Rotate) { TestRotate<CircularList>(); }
TEST(CircularList, Erase) { TestErase<CircularList>(); }
TEST(CircularList, Erase2) { TestErase2<CircularList>(); }
TEST(CircularList, Erase3) { TestErase3<CircularList>(); }
TEST(CircularList, Append) { TestAppend<CircularList>(); }
TEST(CircularList, Merge) { TestMerge<CircularList>(); }
TEST(CircularList, Sort) { TestSort<CircularList>(); }

//---------------------------------------------------------------------------------------------------------------------

TEST(Hierarchy, Init) {
	Object root{"root"};
	
	ASSERT_EQ(root.GetParentNode(), nullptr);
	ASSERT_EQ(root.GetFirstChildNode(), nullptr);
	ASSERT_EQ(root.GetPrevSiblingNode(), nullptr);
	ASSERT_EQ(root.GetNextSiblingNode(), nullptr);
}

//---------------------------------------------------------------------------------------------------------------------

TEST(PoolAllocator, Allocate) {
	using Allocator = PoolAllocator<int, 2>;
	
	Allocator allocator;
	
	auto p = allocator.Allocate<int>();
	void* addr1 = p;
	void* addr2;
	void* addr3;
	
	{
		addr2 = allocator.Allocate<int>();
		EXPECT_NE(addr1, addr2);
		allocator.Deallocate(addr2);
	}
	{
		addr3 = allocator.Allocate<int>();
		EXPECT_NE(addr1, addr3);
		allocator.Deallocate(addr3);
	}
	
	EXPECT_EQ(addr2, addr3);
	
	allocator.Deallocate(p);
	
	auto p1 = allocator.Allocate<int>();
	auto p2 = allocator.Allocate<int>();
	auto p3 = allocator.Allocate<int>();
	
	EXPECT_EQ(addr1, p1);
	EXPECT_EQ(addr2, p2);
	EXPECT_NE(addr3, p3);
	
	auto a1 = Allocator::GetAllocator(p1);
	auto a2 = Allocator::GetAllocator(p2);
	auto a3 = Allocator::GetAllocator(p3);
	
	EXPECT_EQ(a1, std::addressof(allocator));
	EXPECT_EQ(a2, std::addressof(allocator));
	EXPECT_EQ(a3, std::addressof(allocator));
	
	allocator.Deallocate(p1);
	allocator.Deallocate(p2);
	allocator.Deallocate(p3);
}

//---------------------------------------------------------------------------------------------------------------------

TEST(MonotonicAllocator, GetAllocator) {
	using Allocator = MonotonicAllocator<64>;
	
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

TEST(ScopeGuard, Exit) {
	std::string out;
	
	TRY	{
		ON_SCOPE_EXIT_SUCCESS(&out) { out += "1"; };
		ON_SCOPE_EXIT_FAILURE(&out) { out += "2"; };
		ON_SCOPE_EXIT(&out) { out += "3"; };
		
		ScopeGuard guard = [&out] {
			out += "4";
		};
		
		guard.Cancel();
		
		THROW(1);
	}
	CATCH_ALL() {
	}
	
#ifdef __cpp_exceptions
	EXPECT_EQ(out, "32");
#else
	EXPECT_EQ(out, "31");
#endif
}
