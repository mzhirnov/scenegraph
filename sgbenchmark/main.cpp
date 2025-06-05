#include <benchmark/benchmark.h>

#include <scenegraph/ForwardList.h>
#include <scenegraph/PoolAllocator.h>

#include <vector>

class Node : public ForwardListNode<> {
public:
};

static void BM_ForwardListPushFront(benchmark::State& state) {
	const auto size = state.range();
	std::vector<Node> nodes(size);
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
	std::vector<Node> nodes(size);
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
	std::vector<Node> nodes(size);
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
	std::vector<Node> nodes(size);
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
	std::vector<Node> nodes(size);
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
	std::vector<Node> nodes(size);
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
	std::vector<Node*> nodes(size);
	PoolAllocator<Node, 1024> allocator;
	
	for (auto _ : state) {
		for (auto& node : nodes) {
			node = allocator.Allocate();
		}
		
		for (auto& node : nodes) {
			allocator.Deallocate(node);
		}
		
		for (auto& node : nodes) {
			node = allocator.Allocate();
		}
		
		for (auto& node : nodes) {
			allocator.Deallocate(node);
		}
	}
}

BENCHMARK(BM_PoolAllocator)->RangeMultiplier(2)->Range(1 << 8, 1 << 16);

static void BM_MallocFreeAllocator(benchmark::State& state) {
	const auto size = state.range();
	std::vector<Node*> nodes(size);
	
	for (auto _ : state) {
		for (auto& node : nodes) {
			node = (Node*)malloc(sizeof(Node));
		}
		
		for (auto& node : nodes) {
			free(node);
		}
		
		for (auto& node : nodes) {
			node = (Node*)malloc(sizeof(Node));
		}
		
		for (auto& node : nodes) {
			free(node);
		}
	}
}

BENCHMARK(BM_MallocFreeAllocator)->RangeMultiplier(2)->Range(1 << 8, 1 << 16);

static void BM_NewDeleteAllocator(benchmark::State& state) {
	const auto size = state.range();
	std::vector<Node*> nodes(size);
	
	for (auto _ : state) {
		for (auto& node : nodes) {
			node = new Node;
		}
		
		for (auto& node : nodes) {
			delete node;
		}
		
		for (auto& node : nodes) {
			node = new Node;
		}
		
		for (auto& node : nodes) {
			delete node;
		}
	}
}

BENCHMARK(BM_NewDeleteAllocator)->RangeMultiplier(2)->Range(1 << 8, 1 << 16);

BENCHMARK_MAIN();
