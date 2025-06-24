#include <scenegraph/SceneObject.h>
#include <scenegraph/PoolAllocator.h>
#include <scenegraph/MonotonicAllocator.h>
#include <scenegraph/StaticImpl.h>
#include <scenegraph/ScopeGuard.h>

#include <iostream>
#include <memory>
#include <cstdio>

constexpr uint32_t PopCount(uint32_t x) {
	auto a = x - ((x >> 1) & 0x55555555);
	auto b = (((a >> 2) & 0x33333333) + (a & 0x33333333));
	auto c = (((b >> 4) + b) & 0x0f0f0f0f);
	auto d = c + (c >> 8);
	auto e = d + (d >> 16);
	return e & 0x0000003f;
};

constexpr uint32_t Log2(uint32_t x) {
	auto a = x | (x >> 1);
	auto b = a | (a >> 2);
	auto c = b | (b >> 4);
	auto d = c | (c >> 8);
	auto e = d | (d >> 16);
	return PopCount(e >> 1);
};

class HelloComponent : public ComponentImpl<HelloComponent> {
public:
	HelloComponent() = default;
	
private:
	friend class ComponentImpl<HelloComponent>;
	
	void Added(SceneObject*) noexcept {
		puts("Added Hello");
	}
	
	void Removed(SceneObject*) noexcept {
		puts("Removed Hello");
	}
	
	void Apply(SceneObject*) noexcept {
		puts("Hello");
		//Invalidate();
	}
};

class WorldComponent : public ComponentImpl<WorldComponent> {
public:
	WorldComponent() = default;
	
private:
	friend class ComponentImpl<WorldComponent>;
	
	void Added(SceneObject*) noexcept {
		puts("Added World");
	}
	
	void Removed(SceneObject*) noexcept {
		puts("Removed World");
	}
	
	void Apply(SceneObject*) noexcept {
		puts("World");
		//Invalidate();
	}
};

class ExclamationComponent : public ComponentImpl<ExclamationComponent> {
public:
	ExclamationComponent() = default;
	
private:
	friend class ComponentImpl<ExclamationComponent>;
	
	void Added(SceneObject*) noexcept {
		puts("Added Exclamation");
	}
	
	void Removed(SceneObject*) noexcept {
		puts("Removed Exclamation");
	}
	
	void Apply(SceneObject*) noexcept {
		puts("!");
		Remove();
	}
};

class AutoObject {
public:
	AutoObject();
	~AutoObject();
	
	int Value() const;
	
private:
	struct Impl;
	StaticImpl<Impl, 4, alignof(int)> _impl;
};

struct AutoObject::Impl {
	int i = 42;
	
	Impl() = default;
	explicit Impl(int n) noexcept : i(n) {}
	~Impl() = default;
};

AutoObject::AutoObject() : _impl(43) { puts(".ctor"); }
AutoObject::~AutoObject() { puts(".dtor"); }

int AutoObject::Value() const { return _impl->i; }

int main() {
	SceneObject sceneObject;
	HelloComponent c1;
	WorldComponent c2;
	ExclamationComponent c3;
	
	sceneObject.AddComponent(c1);
	sceneObject.AddComponent(c2);
	sceneObject.AddComponent(c3);

	puts("---");
	sceneObject.Update();
	puts("---");
	sceneObject.Update();
	
	{
		PoolAllocator<AutoObject, 2> allocator;
		
		auto p = allocator.Allocate();
		std::cout << p << '\n';
		
		{
			auto obj = std::construct_at(static_cast<AutoObject*>(allocator.Allocate()));
			std::cout << obj << " align: " << alignof(AutoObject) << " size: " << sizeof(AutoObject) << " value: " << obj->Value() << '\n';
			std::destroy_at(obj);
			allocator.Deallocate(obj);
		}
		{
			auto obj = std::construct_at(static_cast<AutoObject*>(allocator.Allocate()));
			std::cout << obj << '\n';
			std::destroy_at(obj);
			allocator.Deallocate(obj);
		}
		
		allocator.Deallocate(p);
		
		void *o1, *o2, *o3;
		
		std::cout << (o1 = allocator.Allocate()) << '\n';
		std::cout << (o2 = allocator.Allocate()) << '\n';
		std::cout << (o3 = allocator.Allocate()) << '\n';
		
		allocator.Deallocate(o1);
		allocator.Deallocate(o2);
		allocator.Deallocate(o3);
	}
	
	TRY	{
		ON_SCOPE_EXIT_SUCCESS() { puts("scope_exit 1"); };
		ON_SCOPE_EXIT_FAILURE() { puts("scope_exit 2"); };
		ON_SCOPE_EXIT() { puts("scope_exit 3"); };
		
		ScopeGuard guard = [] {
			puts("scope_exit 4");
		};
		
		guard.Cancel();
		
		THROW(1);
	}
	CATCH_ALL() {
	}
	
	{
		using Allocator = MonotonicAllocator<64>;
		
		Allocator allocator;
		
		auto* p = allocator.Allocate<int>();
		allocator.Deallocate(p, sizeof(int));
	}
	
	return 0;
}
