#include <scenegraph/SceneObject.h>
#include <cstdio>

class HelloComponent : public ComponentImpl<HelloComponent> {
public:
	HelloComponent() = default;
	
private:
	friend class ComponentImpl<HelloComponent>;
	
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
	
	void Apply(SceneObject*) noexcept {
		puts("!");
		Invalidate();
	}
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
	
	return 0;
}
