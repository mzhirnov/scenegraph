#pragma once

#include <functional>
#include <iostream>
#ifdef __cpp_exceptions
#include <exception>
#endif

template <bool OnSuccess, bool OnFailure>
struct ScopeGuardTag {};

///
/// Scope guard
///
/// Calls a function object on scope exit.
///
template <typename Handler, bool OnSuccess = true, bool OnFailure = true>
class ScopeGuard {
public:
	static_assert(OnSuccess || OnFailure, "At least one of OnSuccess or OnFailure must be true");
	static_assert(std::is_invocable_v<Handler>, "Handler must have signature void()");
	
	ScopeGuard(Handler&& handler)
		: _handler(std::forward<Handler>(handler))
#ifdef __cpp_exceptions
		, _exceptions(std::uncaught_exceptions())
#endif
	{
	}
	
	ScopeGuard(const ScopeGuard&) = delete;
	ScopeGuard& operator=(const ScopeGuard&) = delete;
	
	ScopeGuard(ScopeGuard&&) = delete;
	ScopeGuard& operator=(ScopeGuard&&) = delete;
	
	// Only stack instances allowed
	
	static void* operator new(size_t, ...) = delete;
	static void operator delete(void*, ...) = delete;
	
	static void* operator new[](size_t, ...) = delete;
	static void operator delete[](void*, ...) = delete;
	
	~ScopeGuard() {
		if (_cancelled) {
			return;
		}
		
#ifdef __cpp_exceptions
		const bool exception = std::uncaught_exceptions() != _exceptions;
		
		if ((OnSuccess && !exception) || (OnFailure && exception)) {
			try {
				std::invoke(_handler);
			}
			catch (...) {
				std::cerr << "Exception thrown during scope exit" << std::endl;
				throw;
			}
		}
#else
		if constexpr (OnSuccess) {
			std::invoke(_handler);
		}
#endif
	}
	
	void Cancel() noexcept { _cancelled = true; }

private:
	Handler _handler;
#ifdef __cpp_exceptions
	int _exceptions = 0;
#endif
	bool _cancelled = false;
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
	ScopeGuard UNIQUE_VARIABLE(SCOPE_EXIT_handler_) = [__VA_ARGS__]()
	
#define ON_SCOPE_EXIT_SUCCESS(...) \
	auto UNIQUE_VARIABLE(SCOPE_EXIT_success_handler_) = ScopeGuardTag<true, false>{} + [__VA_ARGS__]()
	
#define ON_SCOPE_EXIT_FAILURE(...) \
	auto UNIQUE_VARIABLE(SCOPE_EXIT_failure_handler_) = ScopeGuardTag<false, true>{} + [__VA_ARGS__]()

#ifdef __cpp_exceptions
	#define TRY try
	#define CATCH(...) catch(__VA_ARGS__)
	#define CATCH_ALL() catch(...)
	#define THROW(...) throw __VA_ARGS__;
#else
	#define TRY
	#define CATCH(...) while (false)
	#define CATCH_ALL() while (false)
	#define THROW(...) (void)0;
#endif
