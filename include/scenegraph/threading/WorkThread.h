#pragma once

#include <atomic>
#include <thread>

struct PipeTask {
	PipeTask* next = nullptr;
	void (*callbackIn)(void*) = nullptr;
	void (*callbackOut)(void*) = nullptr;
	void* param = nullptr;
};

///
/// Pipe of tasks
///
class Pipe {
public:
	Pipe() = default;
	
	void Push(PipeTask* task) noexcept;
	
	PipeTask* Peek() noexcept;
	PipeTask* TryPop() noexcept;
	PipeTask* Pop() noexcept;
	
	void WaitWhile(uint32_t tasks) noexcept;
	void Notify() noexcept;
	
	uint32_t Tasks() const noexcept;

private:
	std::atomic<PipeTask*> _incomingHead = nullptr;
	PipeTask* _outcomingHead = nullptr;
	std::atomic<uint32_t> _tasks{0};
};

///
/// Work thread with tasks queue
///
class WorkThread {
public:
	WorkThread() = default;
	
	~WorkThread();
	
	void Push(PipeTask* task) noexcept;
	PipeTask* TryPop() noexcept;
	
	void WaitOne() noexcept { return WaitN(1); }
	void WaitN(uint32_t n) noexcept;
	void WaitAll() noexcept;
	
	uint32_t TasksIn() const noexcept { return _incoming.Tasks(); }
	uint32_t TasksOut() const noexcept { return _outcoming.Tasks(); }
	
private:
	void ThreadBody() noexcept;

private:
	std::thread _thread{&WorkThread::ThreadBody, this};
	Pipe _incoming;
	Pipe _outcoming;
	std::atomic<bool> _stop{false};
};
