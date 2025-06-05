#include <scenegraph/SceneObject.h>
#include <scenegraph/PoolAllocator.h>
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

struct AutoObject {
	AutoObject() { puts(".ctor"); }
	~AutoObject() { puts(".dtor"); }
	
	int i = 0;
};

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
	
	PoolAllocator<AutoObject, 2> allocator;
	
	auto p = allocator.Allocate();
	std::cout << p << '\n';
	
	{
		auto obj = std::construct_at(allocator.Allocate());
		std::cout << obj << '\n';
		std::destroy_at(obj);
		allocator.Deallocate(obj);
	}
	{
		auto obj = std::construct_at(allocator.Allocate());
		std::cout << obj << '\n';
		std::destroy_at(obj);
		allocator.Deallocate(obj);
	}
	
	allocator.Deallocate(p);
	
	AutoObject *o1, *o2, *o3;
	
	std::cout << (o1 = allocator.Allocate()) << '\n';
	std::cout << (o2 = allocator.Allocate()) << '\n';
	std::cout << (o3 = allocator.Allocate()) << '\n';
	
	allocator.Deallocate(o1);
	allocator.Deallocate(o2);
	allocator.Deallocate(o3);
	
	return 0;
}
