#include <scenegraph/threading/WorkThread.h>

void Pipe::Push(PipeTask* task) noexcept {
	auto tail = task;
	while (tail->next) {
		tail = tail->next;
	}
	
	tail->next = _incomingHead.load(std::memory_order::relaxed);
	while (!_incomingHead.compare_exchange_weak(tail->next, task, std::memory_order::release, std::memory_order::relaxed))
		;
	
	Notify();
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
	
	if (!_outcomingHead) {
		_busy.clear(std::memory_order::relaxed);
	}
	
	return task;
}

bool Pipe::Busy() const noexcept {
	return _busy.test(std::memory_order::relaxed);
}

void Pipe::Wait() noexcept {
	_busy.wait(false);
}

void Pipe::Notify() noexcept {
	_busy.test_and_set(std::memory_order::relaxed);
	_busy.notify_one();
}

//---------------------------------------------------------------------------------------------------------------------

WorkThread::~WorkThread() {
	_stop.test_and_set(std::memory_order::relaxed);
	_incoming.Notify();
	
	if (_thread.joinable()) {
		_thread.join();
	}
	
	while (TryPop())
		;
}

void WorkThread::Push(PipeTask* task) noexcept {
	if (!task) {
		assert(task);
		return;
	}
	
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

void WorkThread::WaitOne() noexcept {
	if (!_outcoming.Busy() && _incoming.Busy()) {
		_outcoming.Wait();
	}
}

void WorkThread::WaitAll() noexcept {
	while (_incoming.Busy()) {
		_outcoming.Wait();
	}
}

void WorkThread::ThreadBody() noexcept {
	do {
		_incoming.Wait();
		
		while (auto task = _incoming.Peek()) {
			if (task->callbackIn) {
				task->callbackIn(task->param);
			}
			_outcoming.Push(_incoming.TryPop());
		}
	}
	while (!_stop.test(std::memory_order::relaxed));
}
