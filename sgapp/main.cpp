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
	DEFINE_COMPONENT_TYPE(HelloComponent)
	
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
	DEFINE_COMPONENT_TYPE(WorldComponent)
	
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
	DEFINE_COMPONENT_TYPE(ExclamationComponent)
	
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
	ComponentFactory<DynamicFactoryPolicy> factory1;
	
	factory1.Register<HelloComponent>();
	factory1.Register<WorldComponent>();
	factory1.Register<ExclamationComponent>();
	
	using ComponentTypes = ComponentTypeList<
		HelloComponent,
		WorldComponent,
		ExclamationComponent
	>;
	
	ComponentFactory<StaticFactoryPolicy<ComponentTypes>> factory2;

	auto scene = std::make_unique<Scene>();
	
	auto sceneObject = scene->AddObject();
	sceneObject.AddComponent(factory1.MakeComponent("HelloComponent", scene.get()));
	sceneObject.AddComponent(factory2.MakeComponent("WorldComponent", scene.get()));
	sceneObject.AddComponent<ExclamationComponent>();
	
	auto child = sceneObject.AppendChild();
	child.AddComponent<ExclamationComponent>();
	
	sceneObject.ForEachComponent<ExclamationComponent>([](SceneObject, ExclamationComponent* c, bool&) {
		std::cout << c << '\n';
	});
	sceneObject.ForEachComponentInChildren<ExclamationComponent>([](SceneObject, ExclamationComponent* c, bool&) {
		std::cout << c << '\n';
	});
	child.ForEachComponentInParent<ExclamationComponent>([](SceneObject, ExclamationComponent* c, bool&) {
		std::cout << c << '\n';
	});
	
	puts("---");
	scene->ForEachObject([](SceneObject sceneObject, bool&) {
		ComponentMessageParams params;
		sceneObject.BroadcastMessage(ComponentMessage::Apply, params);
	});
	puts("---");
	scene->ForEachObject([](SceneObject sceneObject, bool&) {
		ComponentMessageParams params;
		sceneObject.BroadcastMessage(ComponentMessage::Apply, params);
	});
	
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
		allocator.Deallocate(p);
	}
	
	return 0;
}
