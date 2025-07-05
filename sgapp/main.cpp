#include <scenegraph/Scene.h>
#include <scenegraph/Component.h>
#include <scenegraph/ComponentFactory.h>
#include <scenegraph/memory/PoolAllocator.h>
#include <scenegraph/memory/MonotonicAllocator.h>
#include <scenegraph/utils/StaticImpl.h>
#include <scenegraph/utils/ScopeGuard.h>
#include <scenegraph/utils/MurmurHash.h>
#include <scenegraph/utils/BitUtils.h>

#include <iostream>
#include <cstdio>

class HelloComponent : public ComponentImpl<HelloComponent> {
public:
	HelloComponent() { puts("HelloComponent()"); }
	~HelloComponent() { puts("~HelloComponent()"); }
	
private:
	friend class ComponentImpl<HelloComponent>;
	
	void Added(SceneObject*) noexcept { puts("+ Added HelloComponent"); }
	void Removed(SceneObject*) noexcept { puts("- Removed HelloComponent"); }
	
	void Apply(SceneObject*) noexcept {
		puts("Hello");
		//Remove();
	}
};

class WorldComponent : public ComponentImpl<WorldComponent> {
public:
	WorldComponent() { puts("WorldComponent()"); }
	~WorldComponent() { puts("~WorldComponent()"); }
	
private:
	friend class ComponentImpl<WorldComponent>;
	
	void Added(SceneObject*) noexcept { puts("+ Added WorldComponent"); }
	void Removed(SceneObject*) noexcept { puts("- Removed WorldComponent"); }
	
	void Apply(SceneObject*) noexcept {
		puts("World");
		//Remove();
	}
};

class ExclamationComponent : public ComponentImpl<ExclamationComponent> {
public:
	ExclamationComponent() { puts("ExclamationComponent()"); }
	~ExclamationComponent() { puts("~ExclamationComponent()"); }
	
private:
	friend class ComponentImpl<ExclamationComponent>;
	
	void Added(SceneObject*) noexcept { puts("+ Added ExclamationComponent"); }
	void Removed(SceneObject*) noexcept { puts("- Removed ExclamationComponent"); }
	
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
	struct Implementation;
	enum { kImplSize = 4, kImplAlign = alignof(int) };
	
	StaticImpl<Implementation, kImplSize, kImplAlign> _impl;
};

struct AutoObject::Implementation {
	int i = 42;
	
	Implementation() { puts("Implementation()"); }
	explicit Implementation(int n) noexcept : i(n) { puts("Implementation(n)"); }
	~Implementation() { puts("~Implementation()"); }
};

AutoObject::AutoObject() : _impl(43) { puts("AutoObject()"); }
AutoObject::~AutoObject() { puts("~AutoObject()"); }

int AutoObject::Value() const { return _impl->i; }

int main() {
	ComponentFactory factory;
	
	factory.Register(ComponentFactory::HashName("Hello"), HelloComponent::Make);
	factory.Register(ComponentFactory::HashName("World"), WorldComponent::Make);
	factory.Register(ComponentFactory::HashName("Exclamation"), ExclamationComponent::Make);

	auto scene = std::make_unique<Scene>();
	//Scene scene;
	auto sceneObject = scene->AddObject();
	sceneObject.AddComponent(HelloComponent::Make(scene.get()));
	sceneObject.AddComponent(factory.MakeComponent("World", scene.get()));
	sceneObject.AddComponent<ExclamationComponent>();
	
	puts("---");
	sceneObject.SendMessage(ComponentMessage::Apply);
	puts("---");
	sceneObject.SendMessage(ComponentMessage::Apply);
	
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
