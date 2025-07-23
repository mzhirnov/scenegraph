#include <scenegraph/Scene.h>
#include <scenegraph/Component.h>
#include <scenegraph/ComponentFactory.h>
#include <scenegraph/memory/PoolAllocator.h>
#include <scenegraph/memory/MonotonicAllocator.h>
#include <scenegraph/utils/StaticImpl.h>
#include <scenegraph/utils/BitUtils.h>

#include <iostream>
#include <string>
#include <cstdio>

class HelloComponent final : public ComponentImpl<HelloComponent> {
public:
	DEFINE_COMPONENT_TYPE(HelloComponent)
	
	HelloComponent() { puts("HelloComponent()"); }
	~HelloComponent() { puts("~HelloComponent()"); }
	
private:
	friend Super;
	
	void Added(SceneObject) noexcept { puts("+ Added HelloComponent"); }
	void Removed(SceneObject) noexcept { puts("- Removed HelloComponent"); }
	
	void Apply(SceneObject) noexcept {
		puts("Hello");
		//Remove();
	}
};

class WorldComponent final : public ComponentImpl<WorldComponent> {
public:
	DEFINE_COMPONENT_TYPE(WorldComponent)
	
	WorldComponent() { puts("WorldComponent()"); }
	~WorldComponent() { puts("~WorldComponent()"); }
	
private:
	friend Super;
	
	void Added(SceneObject) noexcept { puts("+ Added WorldComponent"); }
	void Removed(SceneObject) noexcept { puts("- Removed WorldComponent"); }
	
	void Apply(SceneObject) noexcept {
		puts("World");
		//Remove();
	}
};

class ExclamationComponent final : public ComponentImpl<ExclamationComponent> {
public:
	DEFINE_COMPONENT_TYPE(ExclamationComponent)
	
	ExclamationComponent() { puts("ExclamationComponent()"); }
	~ExclamationComponent() { puts("~ExclamationComponent()"); }
	
private:
	friend Super;
	
	void Added(SceneObject) noexcept { puts("+ Added ExclamationComponent"); }
	void Removed(SceneObject) noexcept { puts("- Removed ExclamationComponent"); }
	
	void Apply(SceneObject) noexcept {
		puts("!");
		Remove();
	}
};

class NameComponent final : public ComponentImpl<NameComponent> {
public:
	DEFINE_COMPONENT_TYPE(NameComponent)
	
	std::string name;
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
#if 1
	ComponentFactory<DynamicFactoryPolicy> factory1 {
		ComponentInfo<HelloComponent>,
		ComponentInfo<WorldComponent>,
		ComponentInfo<ExclamationComponent>
	};
#else
	ComponentFactory<DynamicFactoryPolicy> factory1;
	
	factory1.Register<HelloComponent>();
	factory1.Register<WorldComponent>();
	factory1.Register<ExclamationComponent>();
#endif
	
	using ComponentTypes = ComponentTypeList<
		HelloComponent,
		WorldComponent,
		ExclamationComponent
	>;
	
	ComponentFactory<StaticFactoryPolicy<ComponentTypes>> factory2;

	auto scene = std::make_unique<Scene>();
	
	{
		auto str = scene->NewString("Hello!");
		std::cout << str->ToStringView() << '\n';
	}
	
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
		sceneObject.BroadcastMessage(ComponentMessages::Apply, params);
	});
	puts("---");
	scene->ForEachObject([](SceneObject sceneObject, bool&) {
		ComponentMessageParams params;
		sceneObject.BroadcastMessage(ComponentMessages::Apply, params);
	});
	
	auto a = sceneObject.AppendChild();
	auto b1 = a.AppendChild();
	auto b2 = a.AppendChild();
	
	a.AddComponent<NameComponent>()->name = "a";
	b1.AddComponent<NameComponent>()->name = "b1";
	b2.AddComponent<NameComponent>()->name = "b2";
	
	b1.AppendChild().AddComponent<NameComponent>()->name = "c1.1";
	b1.AppendChild().AddComponent<NameComponent>()->name = "c1.2";
	b2.AppendChild().AddComponent<NameComponent>()->name = "c2.1";
	b2.AppendChild().AddComponent<NameComponent>()->name = "c2.2";
	
	scene->GetRootObject().WalkChildren(EnumDirection::FirstToLast, EnumCallOrder::PreOrder | EnumCallOrder::PostOrder,
		[](SceneObject sceneObject, EnumCallOrder callOrder, bool& stop) {
			stop = sceneObject.ForEachComponent<NameComponent>([callOrder](SceneObject, NameComponent* c, bool&) {
				std::cout << (callOrder == EnumCallOrder::PreOrder ? "pre " : "post ") << c->name << '\n';
			});
		});
	
	auto fnTestAllocator = [](auto& allocator, const char* name) {
		std::cout << ">>> " << name << '\n';
		
		auto p = allocator.template Allocate<AutoObject>();
		std::cout << p << '\n';
		
		{
			auto obj = std::construct_at(allocator.template Allocate<AutoObject>());
			std::cout << obj << " align: " << alignof(AutoObject) << " size: " << sizeof(AutoObject) << " value: " << obj->Value() << '\n';
			std::destroy_at(obj);
			allocator.Deallocate(obj);
		}
		{
			auto obj = std::construct_at(allocator.template Allocate<AutoObject>());
			std::cout << obj << '\n';
			std::destroy_at(obj);
			allocator.Deallocate(obj);
		}
		
		allocator.Deallocate(p);
		
		void *o1, *o2, *o3;
		
		std::cout << (o1 = allocator.template Allocate<int>()) << '\n';
		std::cout << (o2 = allocator.template Allocate<int>()) << '\n';
		std::cout << (o3 = allocator.template Allocate<int>()) << '\n';
		
		allocator.Deallocate(o1);
		allocator.Deallocate(o2);
		allocator.Deallocate(o3);
		
		std::cout << "<<< " << name << '\n';
	};
	
	{
		PoolAllocator<AutoObject, 2> allocator;
		fnTestAllocator(allocator, "PoolAllocator");
	}
	{
		MonotonicAllocator<128> allocator;
		fnTestAllocator(allocator, "MonotonicAllocator");
	}
	
	}
	}
	
	}
	
	return 0;
}
