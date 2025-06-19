#pragma once

#include <functional>
#include <exception>

template <bool OnSuccess = true, bool OnFailure = true>
struct ScopeGuardTag {};

template <typename Handler, bool OnSuccess = true, bool OnFailure = true, typename = std::enable_if_t<std::is_invocable_v<Handler>>>
class ScopeGuard {
public:
	ScopeGuard(Handler&& handler)
		: _handler(std::forward<Handler>(handler))
		, _exceptions(std::uncaught_exceptions())
	{
	}
	
	ScopeGuard(const ScopeGuard&) = delete;
	ScopeGuard& operator=(const ScopeGuard&) = delete;
	
	ScopeGuard(ScopeGuard&&) = delete;
	ScopeGuard& operator=(ScopeGuard&&) = delete;
	
	~ScopeGuard() {
		int exceptions = std::uncaught_exceptions();
		
		if ((OnSuccess && exceptions == _exceptions) ||
			(OnFailure && exceptions != _exceptions))
		{
			if (!_commit) {
				std::invoke(_handler);
			}
		}
	}
	
	// If commited, handler won't be called
	void Commit() noexcept { _commit = true; }

private:
	Handler _handler;
	int _exceptions = 0;
	bool _commit = false;
};

template <typename Handler, bool OnSuccess, bool OnFailure>
constexpr auto operator+(ScopeGuardTag<OnSuccess, OnFailure>, Handler&& handler) noexcept {
	return ScopeGuard<Handler, OnSuccess, OnFailure>{std::forward<Handler>(handler)};
}

#define CONCAT2(x, y) x##y
#define CONCAT(x, y) CONCAT2(x, y)

#ifdef __COUNTER__
	#define UNIQUE_VARIABLE(x) CONCAT(x, __COUNTER__)
#else
	#define UNIQUE_VARIABLE(x) CONCAT(x, __LINE__)
#endif

#define ON_SCOPE_EXIT(...) \
	auto UNIQUE_VARIABLE(SCOPE_EXIT_handler_) = ScopeGuardTag{} + [__VA_ARGS__]()
	
#define ON_SCOPE_EXIT_SUCCESS(...) \
	auto UNIQUE_VARIABLE(SCOPE_EXIT_success_handler_) = ScopeGuardTag<true, false>{} + [__VA_ARGS__]()
	
#define ON_SCOPE_EXIT_FAILURE(...) \
	auto UNIQUE_VARIABLE(SCOPE_EXIT_failure_handler_) = ScopeGuardTag<false, true>{} + [__VA_ARGS__]()
