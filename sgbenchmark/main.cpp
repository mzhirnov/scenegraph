#include <benchmark/benchmark.h>

#include <scenegraph/linked/ForwardList.h>
#include <scenegraph/memory/PoolAllocator.h>
#include <scenegraph/memory/MonotonicAllocator.h>

#include <vector>

class Node : public ForwardListNode<> {
public:
	alignas(16) float buffer[4];
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
			node = allocator.Allocate<Node>();
		}
		
		for (auto& node : nodes) {
			allocator.Deallocate(node);
		}
		
		for (auto& node : nodes) {
			node = allocator.Allocate<Node>();
		}
		
		for (auto& node : nodes) {
			allocator.Deallocate(node);
		}
	}
}

BENCHMARK(BM_PoolAllocator)->RangeMultiplier(2)->Range(1 << 8, 1 << 16);

static void BM_MonotonicAllocator(benchmark::State& state) {
	const auto size = state.range();
	std::vector<Node*> nodes(static_cast<size_t>(size));
	MonotonicAllocator<16384> allocator;
	
	for (auto _ : state) {
		for (auto& node : nodes) {
			node = allocator.Allocate<Node>();
		}
		
		for (auto& node : nodes) {
			allocator.Deallocate(node);
		}
		
		for (auto& node : nodes) {
			node = allocator.Allocate<Node>();
		}
		
		for (auto& node : nodes) {
			allocator.Deallocate(node);
		}
	}
}

BENCHMARK(BM_MonotonicAllocator)->RangeMultiplier(2)->Range(1 << 8, 1 << 16);

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

BENCHMARK_MAIN();
