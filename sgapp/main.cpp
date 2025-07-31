#include <scenegraph/Scene.h>
#include <scenegraph/Component.h>
#include <scenegraph/ComponentFactory.h>
#include <scenegraph/memory/PoolAllocator.h>
#include <scenegraph/memory/MonotonicAllocator.h>
#include <scenegraph/utils/StaticImpl.h>
#include <scenegraph/utils/BitUtils.h>

#include "Shader.h"

#include "shaders/PositionColor.vert.h"
#include "shaders/SolidColor.frag.h"

#include <iostream>
#include <string>
#include <cstdio>

#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

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

static SDL_GPUDevice* device;
static SDL_Window* window;
static SDL_GPUGraphicsPipeline* pipeline;
static SDL_GPUBuffer* vertexBuffer;
static SDL_GPUTransferBuffer* transferBuffer;

struct PositionColorVertex {
	float x, y, z;
	uint8_t r, g, b, a;
};

extern "C" {

SDL_AppResult SDL_AppInit(void** /*appstate*/, int /*argc*/, char* /*argv*/[]) {
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
	
	SDL_Init(SDL_INIT_VIDEO);
	
	device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_MSL, true, nullptr);
	if (!device) {
		SDL_Log("GPUCreateDevice failed");
		return SDL_APP_FAILURE;
	}
	
	window = SDL_CreateWindow("SGapp", 1280, 720, SDL_WINDOW_RESIZABLE);
	if (!window) {
		SDL_Log("CreateWindow failed: %s", SDL_GetError());
		return SDL_APP_FAILURE;
	}
	
	if (!SDL_ClaimWindowForGPUDevice(device, window)) {
		SDL_Log("GPUClaimWindow failed");
		return SDL_APP_FAILURE;
	}
	
	// Create the shaders
	SDL_GPUShader* vertexShader = LoadShader(device, PositionColor_vert_msl, PositionColor_vert_msl_len, SDL_GPU_SHADERSTAGE_VERTEX);
	if (!vertexShader) {
		SDL_Log("Failed to create vertex shader!");
		return SDL_APP_FAILURE;
	}

	SDL_GPUShader* fragmentShader = LoadShader(device, SolidColor_frag_msl, SolidColor_frag_msl_len, SDL_GPU_SHADERSTAGE_FRAGMENT);
	if (!fragmentShader) {
		SDL_Log("Failed to create fragment shader!");
		return SDL_APP_FAILURE;
	}

	// Create the pipeline
	SDL_GPUColorTargetDescription targetDescriptions[1] = {{
		.format = SDL_GetGPUSwapchainTextureFormat(device, window),
		.blend_state = {
			.enable_blend = true,
			.color_blend_op = SDL_GPU_BLENDOP_ADD,
			.alpha_blend_op = SDL_GPU_BLENDOP_ADD,
			.src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA,
			.dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
			.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA,
			.dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
		}
	}};
	
	SDL_GPUVertexBufferDescription vertexBufferDescriptions[1] = {{
		.slot = 0,
		.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
		.instance_step_rate = 0,
		.pitch = sizeof(PositionColorVertex)
	}};
	
	SDL_GPUVertexAttribute vertexAttributes[2] = {{
		.buffer_slot = 0,
		.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
		.location = 0,
		.offset = 0
	}, {
		.buffer_slot = 0,
		.format = SDL_GPU_VERTEXELEMENTFORMAT_UBYTE4_NORM,
		.location = 1,
		.offset = sizeof(float) * 3
	}};
	
	SDL_GPUGraphicsPipelineCreateInfo pipelineCreateInfo = {
		.target_info = {
			.num_color_targets = SDL_arraysize(targetDescriptions),
			.color_target_descriptions = targetDescriptions,
		},
		// This is set up to match the vertex shader layout!
		.vertex_input_state = {
			.num_vertex_buffers = SDL_arraysize(vertexBufferDescriptions),
			.vertex_buffer_descriptions = vertexBufferDescriptions,
			.num_vertex_attributes = SDL_arraysize(vertexAttributes),
			.vertex_attributes = vertexAttributes
		},
		.primitive_type = SDL_GPU_PRIMITIVETYPE_LINELIST,
		.vertex_shader = vertexShader,
		.fragment_shader = fragmentShader
	};
	
	pipeline = SDL_CreateGPUGraphicsPipeline(device, &pipelineCreateInfo);
	
	// Clean up shader resources
	SDL_ReleaseGPUShader(device, vertexShader);
	SDL_ReleaseGPUShader(device, fragmentShader);
	
	// Create the vertex buffer
	SDL_GPUBufferCreateInfo bufferCreateInfo = {
		.usage = SDL_GPU_BUFFERUSAGE_VERTEX,
		.size = sizeof(PositionColorVertex) * 2048
	};
	vertexBuffer = SDL_CreateGPUBuffer(device, &bufferCreateInfo);
	
	// To get data into the vertex buffer, we have to use a transfer buffer
	SDL_GPUTransferBufferCreateInfo transferBufferCreateInfo = {
		.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
		.size = sizeof(PositionColorVertex) * 2048
	};
	transferBuffer = SDL_CreateGPUTransferBuffer(device, &transferBufferCreateInfo);
	
	return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* /*appstate*/, SDL_Event* event) {
	switch (event->type) {
	case SDL_EVENT_QUIT:
		return SDL_APP_SUCCESS;
	}
	
	return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* /*appstate*/) {
    auto cmdbuf = SDL_AcquireGPUCommandBuffer(device);
    if (!cmdbuf) {
        SDL_Log("AcquireGPUCommandBuffer failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
	
    SDL_GPUTexture* swapchainTexture;
    if (!SDL_WaitAndAcquireGPUSwapchainTexture(cmdbuf, window, &swapchainTexture, nullptr, nullptr)) {
        SDL_Log("WaitAndAcquireGPUSwapchainTexture failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
	
	if (swapchainTexture) {
		// Write geometry to the transfer data
		auto vertices = static_cast<PositionColorVertex*>(SDL_MapGPUTransferBuffer(device, transferBuffer, true));
		*vertices++ = { -0.5f,  0.5f, 0, 255,   0, 255, 255 };
		*vertices++ = { -0.5f, -0.5f, 0, 255,   0, 255, 255 };
		*vertices++ = { -0.5f,  0.5f, 0, 255,   0,   0, 255 };
		*vertices++ = {  0.5f,  0.5f, 0, 255,   0,   0, 255 };
		*vertices++ = {  0.5f,  0.5f, 0, 255, 255,   0, 255 };
		*vertices++ = {  0.5f, -0.5f, 0, 255, 255,   0, 255 };
		*vertices++ = {  0.5f, -0.5f, 0,   0, 255, 255, 255 };
		*vertices++ = { -0.5f, -0.5f, 0,   0, 255, 255, 255 };
		SDL_UnmapGPUTransferBuffer(device, transferBuffer);
		
		// Upload the transfer data to the vertex buffer
		SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(cmdbuf);
		SDL_GPUTransferBufferLocation transferBufferLocation = {
			.transfer_buffer = transferBuffer,
			.offset = 0
		};
		SDL_GPUBufferRegion bufferRegion = {
			.buffer = vertexBuffer,
			.offset = 0,
			.size = sizeof(PositionColorVertex) * 8
		};
		SDL_UploadToGPUBuffer(copyPass, &transferBufferLocation, &bufferRegion, true);
		SDL_EndGPUCopyPass(copyPass);
		
		// Render geometry
		SDL_GPUColorTargetInfo colorTargetInfos[1] = {{
			.texture = swapchainTexture,
			.clear_color = SDL_FColor{ 0.0f, 0.0f, 0.0f, 1.0f },
			.load_op = SDL_GPU_LOADOP_CLEAR,
			.store_op = SDL_GPU_STOREOP_STORE,
		}};
		SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(cmdbuf, colorTargetInfos, SDL_arraysize(colorTargetInfos), nullptr);
		SDL_BindGPUGraphicsPipeline(renderPass, pipeline);
		SDL_GPUBufferBinding bufferBindings[1] = {{
			.buffer = vertexBuffer,
			.offset = 0
		}};
		SDL_BindGPUVertexBuffers(renderPass, 0, bufferBindings, SDL_arraysize(bufferBindings));
		SDL_DrawGPUPrimitives(renderPass, 8, 1, 0, 0);
		SDL_EndGPURenderPass(renderPass);
	}
	
	SDL_SubmitGPUCommandBuffer(cmdbuf);
	
	return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* /*appstate*/, SDL_AppResult /*result*/) {
	SDL_ReleaseGPUTransferBuffer(device, transferBuffer);
	SDL_ReleaseGPUBuffer(device, vertexBuffer);
	SDL_ReleaseGPUGraphicsPipeline(device, pipeline);
	SDL_ReleaseWindowFromGPUDevice(device, window);
	SDL_DestroyWindow(window);
	SDL_DestroyGPUDevice(device);
}

} // extern "C"
