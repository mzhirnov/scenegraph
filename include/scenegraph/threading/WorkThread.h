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
/// Queue of tasks
///
class Pipe {
public:
	Pipe() = default;
	
	void Push(PipeTask* task) noexcept;
	
	PipeTask* Peek() noexcept;
	PipeTask* TryPop() noexcept;
	
	bool Busy() const noexcept;
	
	void Wait() noexcept;
	void Notify() noexcept;

private:
	std::atomic<PipeTask*> _incomingHead = nullptr;
	PipeTask* _outcomingHead = nullptr;
	std::atomic_flag _busy = ATOMIC_FLAG_INIT;
};

///
/// Work thread with task queue
///
class WorkThread {
public:
	WorkThread() = default;
	
	~WorkThread();
	
	void Push(PipeTask* task) noexcept;
	PipeTask* TryPop() noexcept;
	
	void WaitOne() noexcept;
	void WaitAll() noexcept;
	
private:
	void ThreadBody() noexcept;

private:
	Pipe _incoming;
	Pipe _outcoming;
	std::thread _thread{&WorkThread::ThreadBody, this};
	std::atomic_flag _stop = ATOMIC_FLAG_INIT;
};
