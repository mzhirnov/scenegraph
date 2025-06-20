#include <benchmark/benchmark.h>

#include <scenegraph/ForwardList.h>
#include <scenegraph/PoolAllocator.h>

#include <vector>
//#include <experimental/memory_resource>

class Node : public ForwardListNode<> {
public:
};

static void BM_ForwardListPushFront(benchmark::State& state) {
	const auto size = state.range();
	std::vector<Node> nodes(static_cast<size_t>(size));
	ForwardList<Node> list;
	for (auto _ : state) {
		for (auto& node : nodes) {
			list.PushFront(node);
		}
	}
}
BENCHMARK(BM_ForwardListPushFront)->RangeMultiplier(2)->Range(1, 32);

static void BM_CircularForwardListPushFront(benchmark::State& state) {
	const auto size = state.range();
	std::vector<Node> nodes(static_cast<size_t>(size));
	CircularForwardList<Node> list;
	for (auto _ : state) {
		for (auto& node : nodes) {
			list.PushFront(node);
		}
	}
}
BENCHMARK(BM_CircularForwardListPushFront)->RangeMultiplier(2)->Range(1, 32);

static void BM_ForwardListPushBack(benchmark::State& state) {
	const auto size = state.range();
	std::vector<Node> nodes(static_cast<size_t>(size));
	ForwardList<Node> list;
	for (auto _ : state) {
		for (auto& node : nodes) {
			list.PushBack(node);
		}
	}
}
BENCHMARK(BM_ForwardListPushBack)->RangeMultiplier(2)->Range(1, 32);

static void BM_CircularForwardListPushBack(benchmark::State& state) {
	const auto size = state.range();
	std::vector<Node> nodes(static_cast<size_t>(size));
	CircularForwardList<Node> list;
	for (auto _ : state) {
		for (auto& node : nodes) {
			list.PushBack(node);
		}
	}
}
BENCHMARK(BM_CircularForwardListPushBack)->RangeMultiplier(2)->Range(1, 32);

static void BM_ForwardListIterate(benchmark::State& state) {
	const auto size = state.range();
	std::vector<Node> nodes(static_cast<size_t>(size));
	ForwardList<Node> list;
	for (auto& node : nodes) {
		list.PushBack(node);
	}
	
	for (auto _ : state) {
		for (auto& node : list) {
			benchmark::DoNotOptimize(node);
		}
	}
}
BENCHMARK(BM_ForwardListIterate)->RangeMultiplier(2)->Range(1, 32);

static void BM_CircularForwardListIterate(benchmark::State& state) {
	const auto size = state.range();
	std::vector<Node> nodes(static_cast<size_t>(size));
	CircularForwardList<Node> list;
	for (auto& node : nodes) {
		list.PushBack(node);
	}
	
	for (auto _ : state) {
		for (auto& node : list) {
			benchmark::DoNotOptimize(node);
		}
	}
}
BENCHMARK(BM_CircularForwardListIterate)->RangeMultiplier(2)->Range(1, 32);

static void BM_PoolAllocator(benchmark::State& state) {
	const auto size = state.range();
	std::vector<Node*> nodes(static_cast<size_t>(size));
	PoolAllocator<Node, 1024> allocator;
	
	for (auto _ : state) {
		for (auto& node : nodes) {
			node = static_cast<Node*>(allocator.Allocate());
		}
		
		for (auto& node : nodes) {
			allocator.Deallocate(node);
		}
		
		for (auto& node : nodes) {
			node = static_cast<Node*>(allocator.Allocate());
		}
		
		for (auto& node : nodes) {
			allocator.Deallocate(node);
		}
	}
}

BENCHMARK(BM_PoolAllocator)->RangeMultiplier(2)->Range(1 << 8, 1 << 16);

static void BM_StdAllocator(benchmark::State& state) {
	const auto size = state.range();
	std::vector<Node*> nodes(static_cast<size_t>(size));
	std::allocator<Node> allocator;
	
	for (auto _ : state) {
		for (auto& node : nodes) {
			node = allocator.allocate(1);
		}
		
		for (auto& node : nodes) {
			allocator.deallocate(node, 1);
		}
		
		for (auto& node : nodes) {
			node = allocator.allocate(1);
		}
		
		for (auto& node : nodes) {
			allocator.deallocate(node, 1);
		}
	}
}

BENCHMARK(BM_StdAllocator)->RangeMultiplier(2)->Range(1 << 8, 1 << 16);

//static void BM_StdPoolAllocator(benchmark::State& state) {
//	const auto size = state.range();
//	std::vector<Node*> nodes(size);
//	std::experimental::pmr::unsynchronized_pool_resource allocator;
//
//	for (auto _ : state) {
//		for (auto& node : nodes) {
//			node = (Node*)allocator.allocate(sizeof(Node));
//		}
//
//		for (auto& node : nodes) {
//			allocator.deallocate(node);
//		}
//
//		for (auto& node : nodes) {
//			node = (Node*)allocator.allocate(sizeof(Node));
//		}
//
//		for (auto& node : nodes) {
//			allocator.deallocate(node);
//		}
//	}
//}
//
//BENCHMARK(BM_StdPoolAllocator)->RangeMultiplier(2)->Range(1 << 8, 1 << 16);

BENCHMARK_MAIN();
