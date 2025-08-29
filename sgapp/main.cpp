#include <scenegraph/ApplicationContext.h>
#include <scenegraph/SystemContext.h>
#include <scenegraph/Scene.h>
#include <scenegraph/Component.h>
#include <scenegraph/ComponentFactory.h>
#include <scenegraph/memory/PoolAllocator.h>
#include <scenegraph/memory/MonotonicAllocator.h>
#include <scenegraph/utils/StaticImpl.h>
#include <scenegraph/utils/BitUtils.h>
#include <scenegraph/utils/ScopeGuard.h>
#include <scenegraph/math/Matrix4.h>
#include <scenegraph/math/Matrix32.h>
#include <scenegraph/components/Transform2DComponent.h>
#include <scenegraph/render/Vertex.h>
#include <scenegraph/threading/WorkThread.h>

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

///
/// Application context
///
class Application final : public ApplicationContext {
public:
	Application() {
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
		sceneObject.AddComponent<Transform2DComponent>();
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
			
			allocator.Deallocate(o3);
			allocator.Deallocate(o2);
			allocator.Deallocate(o1);
			
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
		
		{
			auto ms = Matrix32MakeScale(0.8f, 0.8f);
			auto mx = Matrix32MakeXShear(0.7f);
			auto mr = Matrix32MakeRotation(3.1415f / 4);
			auto mt = Matrix32MakeTranslation(100, 100);
			
			auto m3 = ms * mx * mr * mt;
			
			std::cout << "ms * mx * mr * mt = " << m3.a << ' ' << m3.b << ' ' << m3.c << ' ' << m3.d << ' ' << m3.tx << ' ' << m3.ty << '\n';
			
			auto m4 = Matrix32Invert(m3, nullptr);
			
			std::cout << "m inv = " << m4.a << ' ' << m4.b << ' ' << m4.c << ' ' << m4.d << ' ' << m4.tx << ' ' << m4.ty << '\n';
			
			auto m5 = m3 * m4;
			
			std::cout << "m * m inv = " << m5.a << ' ' << m5.b << ' ' << m5.c << ' ' << m5.d << ' ' << m5.tx << ' ' << m5.ty << '\n';
			
			auto m6 = m4 * m3;
			
			std::cout << "m inv * m = " << m6.a << ' ' << m6.b << ' ' << m6.c << ' ' << m6.d << ' ' << m6.tx << ' ' << m6.ty << '\n';
			
			auto m7 = Matrix32MakeWithTransform2D({.sx = 0.8f, .sy = 0.8f, .shearX = 0.7f, .rad = 3.1415f / 4, .tx = 100, .ty = 100});
			
			std::cout << "m tr = " << m7.a << ' ' << m7.b << ' ' << m7.c << ' ' << m7.d << ' ' << m7.tx << ' ' << m7.ty << '\n';
			
			Transform2D c;
			[[maybe_unused]] bool invertible = Matrix32DecomposeToTransform2D(m3, &c);
			assert(invertible);
			
			std::cout << "decompose = " << c.sx << ' ' << c.sy << ' ' << c.shearX << ' ' << c.shearY << ' ' << c.rad << ' ' << c.tx << ' ' << c.ty << '\n';
		}
		
		{
			struct TaskData {
				std::string in = "Hello, ";
				std::string out;
			} data;
			
			PipeTask task = {
				.callbackIn = +[](void* param) {
					auto data = static_cast<TaskData*>(param);
					data->out = data->in + "World!";
				},
				.callbackOut = +[](void* param) {
					auto data = static_cast<TaskData*>(param);
					std::cout << data->out << '\n';
				},
				.param = &data
			};
			
			WorkThread thread;
			std::cout << thread.TasksIn() << ' ' << thread.TasksOut() << '\n';
			thread.Push(&task);
			std::cout << thread.TasksIn() << ' ' << thread.TasksOut() << '\n';
			thread.WaitOne();
			std::cout << thread.TasksIn() << ' ' << thread.TasksOut() << '\n';
			
			while (thread.TryPop()) {
				std::cout << thread.TasksIn() << ' ' << thread.TasksOut() << '\n';
			}
		}
	}

private:
	virtual const char* GetApplicationName() override {
		return "SGapp";
	}
	
	virtual bool Initialize(int, char*[]) override {
		assert(!_initialized);
		if (_initialized) {
			return true;
		}
		
		_initialized = true;
		return true;
	}
	
	virtual void Finalize() override {
		_instance.reset();
	}
	
public:
	static Application* GetInstance() noexcept {
		if (!_instance) {
			_instance = std::make_unique<Application>();
		}
		return _instance.get();
	}
	
private:
	bool _initialized = false;
	
	inline static std::unique_ptr<Application> _instance;
};

ApplicationContext* GetApplicationContext() {
	return Application::GetInstance();
}
