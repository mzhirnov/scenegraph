#include <scenegraph/threading/WorkThread.h>

void Pipe::Push(PipeTask* task) noexcept {
	assert(task != nullptr);
	assert(task->callbackIn != nullptr);
	
	task->next = _incomingHead.load(std::memory_order::relaxed);
	while (!_incomingHead.compare_exchange_weak(task->next, task, std::memory_order::release, std::memory_order::relaxed))
		;
	
	_tasks.fetch_add(1, std::memory_order::relaxed);
}

PipeTask* Pipe::Peek() noexcept {
	if (!_outcomingHead) {
		if (!(_outcomingHead = _incomingHead.exchange(nullptr, std::memory_order::relaxed))) {
			return nullptr;
		}
		// Reverse list
		PipeTask* prev = nullptr;
		PipeTask* current = _outcomingHead;
		while (current) {
			auto next = current->next;
			current->next = prev;
			prev = current;
			current = next;
		}
		_outcomingHead = prev;
	}
	
	return _outcomingHead;
}

PipeTask* Pipe::TryPop() noexcept {
	if (!Peek()) {
		return nullptr;
	}
	
	auto task = _outcomingHead;
	_outcomingHead = _outcomingHead->next;
	task->next = nullptr;
	
	_tasks.fetch_sub(1, std::memory_order::relaxed);
	
	return task;
}

PipeTask* Pipe::Pop() noexcept {
	_tasks.wait(0);
	return TryPop();
}

void Pipe::WaitWhile(uint32_t tasks) noexcept {
	_tasks.wait(tasks);
}

void Pipe::Notify() noexcept {
	_tasks.fetch_add(1, std::memory_order::release);
	_tasks.notify_one();
	_tasks.fetch_sub(1, std::memory_order::relaxed);
}

uint32_t Pipe::Tasks() const noexcept {
	return _tasks.load(std::memory_order::relaxed);
}

//---------------------------------------------------------------------------------------------------------------------

WorkThread::~WorkThread() {
	_stop.store(true, std::memory_order::relaxed);
	_incoming.Notify();
	
	if (_thread.joinable()) {
		_thread.join();
	}
}

void WorkThread::Push(PipeTask* task) noexcept {
	_incoming.Push(task);
}

PipeTask* WorkThread::TryPop() noexcept {
	if (auto task = _outcoming.Peek()) {
		if (task->callbackOut) {
			task->callbackOut(task->param);
		}
	}
	return _outcoming.TryPop();
}

void WorkThread::WaitN(uint32_t n) noexcept {
	// Wait until there are at most n outcoming tasks
	for (auto i = _outcoming.Tasks(); i < n; i++) {
		auto tasks = _incoming.Tasks();
		if (!tasks) {
			break;
		}
		_incoming.WaitWhile(tasks);
	}
}

void WorkThread::WaitAll() noexcept {
	// Wait until there are incoming tasks
	while (auto tasks = _incoming.Tasks()) {
		_incoming.WaitWhile(tasks);
	}
}

void WorkThread::ThreadBody() noexcept {
	while (!_stop.load(std::memory_order::relaxed)) {
		// Wait until there'll be at least one incoming task
		_incoming.WaitWhile(0);
		
		while (auto task = _incoming.Peek()) {
			task->callbackIn(task->param);
			_outcoming.Push(_incoming.TryPop());
		}
	}
}
