#include <benchmark/benchmark.h>

#include <scenegraph/ForwardList.h>

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
BENCHMARK(BM_ForwardListPushFront)->RangeMultiplier(2)->Range(2, 128);

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
BENCHMARK(BM_CircularForwardListPushFront)->RangeMultiplier(2)->Range(2, 128);

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
BENCHMARK(BM_ForwardListPushBack)->RangeMultiplier(2)->Range(2, 128);

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
BENCHMARK(BM_CircularForwardListPushBack)->RangeMultiplier(2)->Range(2, 128);

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
BENCHMARK(BM_ForwardListIterate)->RangeMultiplier(2)->Range(2, 128);

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
BENCHMARK(BM_CircularForwardListIterate)->RangeMultiplier(2)->Range(2, 128);

BENCHMARK_MAIN();
