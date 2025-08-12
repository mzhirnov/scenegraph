#include <scenegraph/Scene.h>
#include <scenegraph/Component.h>
#include <scenegraph/ComponentFactory.h>
#include <scenegraph/memory/PoolAllocator.h>
#include <scenegraph/memory/MonotonicAllocator.h>
#include <scenegraph/utils/StaticImpl.h>
#include <scenegraph/utils/BitUtils.h>
#include <scenegraph/geometry/Matrix4.h>
#include <scenegraph/components/Transform2DComponent.h>


#include "shaders/PositionColorTransform.vert.h"
#include "shaders/PullSpriteBatch.vert.h"
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

static SDL_Window* window;
static SDL_GPUDevice* device;
static SDL_GPUGraphicsPipeline* pipeline;
static SDL_GPUGraphicsPipeline* spritePipeline;
static SDL_GPUBuffer* vertexBuffer;
static SDL_GPUBuffer* spriteDataBuffer;
static SDL_GPUTransferBuffer* transferBuffer;
static SDL_GPUTransferBuffer* spriteDataTransferBuffer;

struct SpriteInstance {
	Matrix32 transform;
	float pivotX;
	float pivotY;
	float r, g, b, a;
	float texU, texV, texW, texH;
};

template <std::size_t Stage = 0>
struct Position3 {
	float x, y, z;
};

template <std::size_t Stage = 0>
struct Color {
	uint8_t r, g, b, a;
};

template <typename T>
constexpr SDL_GPUVertexElementFormat VertexElementFormat = SDL_GPU_VERTEXELEMENTFORMAT_INVALID;

template <std::size_t Stage> constexpr SDL_GPUVertexElementFormat VertexElementFormat<Position3<Stage>> = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
template <std::size_t Stage> constexpr SDL_GPUVertexElementFormat VertexElementFormat<Color<Stage>> = SDL_GPU_VERTEXELEMENTFORMAT_UBYTE4_NORM;

template <typename... Attr>
struct Vertex : Attr... {
	static constexpr uint32_t Size() noexcept { return sizeof(Vertex); }
	
	template <typename Callback>
	constexpr void ForEachAttribute(Callback&& callback) noexcept {
		(std::invoke(std::forward<Callback>(callback), this, static_cast<Attr*>(this)), ...);
	}
};

using PositionColorVertex = Vertex<Position3<>, Color<>>;

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
	
	SDL_Init(SDL_INIT_VIDEO);
	
	window = SDL_CreateWindow("SGapp", 1280, 720, SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);
	if (!window) {
		SDL_Log("CreateWindow failed: %s", SDL_GetError());
		return SDL_APP_FAILURE;
	}
	
	device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_MSL | SDL_GPU_SHADERFORMAT_METALLIB, true, nullptr);
	if (!device) {
		SDL_Log("GPUCreateDevice failed");
		return SDL_APP_FAILURE;
	}
	
	if (!SDL_ClaimWindowForGPUDevice(device, window)) {
		SDL_Log("GPUClaimWindow failed");
		return SDL_APP_FAILURE;
	}
	
	// Create the shaders
	SDL_GPUShaderCreateInfo vertexShaderCreateInfo = {
		.code = PositionColorTransform_vert_air,
		.code_size = PositionColorTransform_vert_air_len,
		.format = SDL_GPU_SHADERFORMAT_METALLIB,
		.stage = SDL_GPU_SHADERSTAGE_VERTEX,
		.num_uniform_buffers = 1
	};
	SDL_GPUShader* vertexShader = SDL_CreateGPUShader(device, &vertexShaderCreateInfo);
	if (!vertexShader) {
		SDL_Log("Failed to create vertex shader!");
		return SDL_APP_FAILURE;
	}
	
	SDL_GPUShaderCreateInfo spriteVertexShaderCreateInfo = {
		.code = PullSpriteBatch_vert_air,
		.code_size = PullSpriteBatch_vert_air_len,
		.format = SDL_GPU_SHADERFORMAT_METALLIB,
		.stage = SDL_GPU_SHADERSTAGE_VERTEX,
		.num_uniform_buffers = 1,
		.num_storage_buffers = 1
	};
	SDL_GPUShader* spriteVertexShader = SDL_CreateGPUShader(device, &spriteVertexShaderCreateInfo);
	if (!spriteVertexShader) {
		SDL_Log("Failed to create vertex shader!");
		return SDL_APP_FAILURE;
	}

	SDL_GPUShaderCreateInfo fragmentShaderCreateInfo = {
		.code = SolidColor_frag_air,
		.code_size = SolidColor_frag_air_len,
		.format = SDL_GPU_SHADERFORMAT_METALLIB,
		.stage = SDL_GPU_SHADERSTAGE_FRAGMENT
	};
	SDL_GPUShader* fragmentShader = SDL_CreateGPUShader(device, &fragmentShaderCreateInfo);
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
			.src_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE,
			.dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
			.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE,
			.dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
		}
	}};
	
	SDL_GPUVertexBufferDescription vertexBufferDescriptions[1] = {{
		.slot = 0,
		.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
		.instance_step_rate = 0,
		.pitch = PositionColorVertex::Size()
	}};
	
	SDL_GPUVertexAttribute vertexAttributes[2];
	
	PositionColorVertex{}.ForEachAttribute([&vertexAttributes, i = 0u](auto* v, auto* attr) mutable {
		using Attr = std::remove_cvref_t<std::remove_pointer_t<decltype(attr)>>;
		
		static_assert(VertexElementFormat<Attr> != SDL_GPU_VERTEXELEMENTFORMAT_INVALID);
		
		vertexAttributes[i].location = i;
		vertexAttributes[i].buffer_slot = 0;
		vertexAttributes[i].format = VertexElementFormat<Attr>;
		vertexAttributes[i].offset = static_cast<Uint32>(reinterpret_cast<std::byte*>(attr) - reinterpret_cast<std::byte*>(v));
		
		i++;
	});
	
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
	if (!pipeline) {
        return SDL_APP_FAILURE;
    }
	
	SDL_GPUGraphicsPipelineCreateInfo spritePipelineCreateInfo = {
		.target_info = {
			.num_color_targets = SDL_arraysize(targetDescriptions),
			.color_target_descriptions = targetDescriptions,
		},
		.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
		.vertex_shader = spriteVertexShader,
		.fragment_shader = fragmentShader
	};
	
	spritePipeline = SDL_CreateGPUGraphicsPipeline(device, &spritePipelineCreateInfo);
	if (!spritePipeline) {
        return SDL_APP_FAILURE;
    }
	
	// Clean up shader resources
	SDL_ReleaseGPUShader(device, vertexShader);
	SDL_ReleaseGPUShader(device, spriteVertexShader);
	SDL_ReleaseGPUShader(device, fragmentShader);
	
	// Create the vertex buffer
	SDL_GPUBufferCreateInfo bufferCreateInfo = {
		.usage = SDL_GPU_BUFFERUSAGE_VERTEX,
		.size = PositionColorVertex::Size() * 2048
	};
	vertexBuffer = SDL_CreateGPUBuffer(device, &bufferCreateInfo);
	
	// To get data into the vertex buffer, we have to use a transfer buffer
	SDL_GPUTransferBufferCreateInfo transferBufferCreateInfo = {
		.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
		.size = PositionColorVertex::Size() * 2048
	};
	transferBuffer = SDL_CreateGPUTransferBuffer(device, &transferBufferCreateInfo);
	
	SDL_GPUTransferBufferCreateInfo spriteTransferBufferCreateInfo = {
		.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
		.size = sizeof(SpriteInstance) * 2048
	};
	spriteDataTransferBuffer = SDL_CreateGPUTransferBuffer(device, &spriteTransferBufferCreateInfo);
	
	SDL_GPUBufferCreateInfo spriteBufferCreateInfo = {
		.usage = SDL_GPU_BUFFERUSAGE_GRAPHICS_STORAGE_READ,
		.size = sizeof(SpriteInstance) * 2048
	};
	spriteDataBuffer = SDL_CreateGPUBuffer(device, &spriteBufferCreateInfo);
	
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
    Uint32 width, height;
    if (!SDL_WaitAndAcquireGPUSwapchainTexture(cmdbuf, window, &swapchainTexture, &width, &height)) {
        SDL_Log("WaitAndAcquireGPUSwapchainTexture failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    
    float scale = SDL_GetWindowPixelDensity(window);
    width = static_cast<Uint32>(width / scale);
    height = static_cast<Uint32>(height / scale);
	
	if (swapchainTexture) {
		Matrix4 viewProjectionMatrix = Matrix4MakeOrthographicOffCenter(0, width, height, 0, 0.0f, 1.0f);
		SDL_PushGPUVertexUniformData(cmdbuf, 0, &viewProjectionMatrix, sizeof(Matrix4));
		
		{
			float l = 0.5f;
			float r = width;
			float t = 0.5f;
			float b = height;
			
			// Write geometry to the transfer data
			auto vertices = static_cast<PositionColorVertex*>(SDL_MapGPUTransferBuffer(device, transferBuffer, true));
			*vertices++ = { { l, b, 0 }, { 255, 255, 255, 255 } };
			*vertices++ = { { l, t, 0 }, { 255, 255, 255, 255 } };
			*vertices++ = { { l, t, 0 }, {   0, 255,   0, 255 } };
			*vertices++ = { { r, t, 0 }, {   0, 255,   0, 255 } };
			*vertices++ = { { r, t, 0 }, {   0,   0, 255, 255 } };
			*vertices++ = { { r, b, 0 }, {   0,   0, 255, 255 } };
			*vertices++ = { { r, b, 0 }, { 255,   0,   0, 255 } };
			*vertices++ = { { l, b, 0 }, { 255,   0,   0, 255 } };
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
				.size = PositionColorVertex::Size() * 8
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
		
		{
			constexpr int kNumSprites = 21;
			SpriteInstance* dataPtr = static_cast<SpriteInstance*>(SDL_MapGPUTransferBuffer(device, spriteDataTransferBuffer, true));
			for (int i = 0; i < kNumSprites - 1; i++) {
				Transform2D c = {.sx = 32, .sy = 32, .shearX = 0.5f, .rad = SDL_PI_F / (kNumSprites - 1) * i, .tx = 100 + i * 50.0f, .ty = 100};
				dataPtr[i].transform = Matrix32MakeWithTransform2D(c);
				dataPtr[i].pivotX = 0.5f;
				dataPtr[i].pivotY = 0.5f;
				dataPtr[i].r = (255 - 10 * i) / 255.0f;
				dataPtr[i].g = (255 - 5 * i) / 255.0f;
				dataPtr[i].b = 1.0f;
				dataPtr[i].a = 1.0f;
			}
			Transform2D c = {.sx = 32, .sy = 32, /*.rad = SDL_PI_F / 8,*/ .tx = 100, .ty = 100};
			dataPtr[20].transform = Matrix32MakeWithTransform2D(c);
			dataPtr[20].pivotX = 0.5f;
			dataPtr[20].pivotY = 0.5f;
			dataPtr[20].r = 1.0f;
			dataPtr[20].g = 0;
			dataPtr[20].b = 0;
			dataPtr[20].a = 1.0f;
			SDL_UnmapGPUTransferBuffer(device, spriteDataTransferBuffer);
			
			// Upload instance data
			SDL_GPUTransferBufferLocation location = {
				.transfer_buffer = spriteDataTransferBuffer,
				.offset = 0
			};
			SDL_GPUBufferRegion region = {
				.buffer = spriteDataBuffer,
				.offset = 0,
				.size = kNumSprites * sizeof(SpriteInstance)
			};
			SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(cmdbuf);
			SDL_UploadToGPUBuffer(copyPass, &location, &region, true);
			SDL_EndGPUCopyPass(copyPass);
			
			// Render sprites
			SDL_GPUColorTargetInfo targetInfo = {
				.texture = swapchainTexture,
				.load_op = SDL_GPU_LOADOP_LOAD,
				.store_op = SDL_GPU_STOREOP_STORE
			};
			SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(cmdbuf, &targetInfo, 1, nullptr);
			SDL_BindGPUGraphicsPipeline(renderPass, spritePipeline);
			SDL_BindGPUVertexStorageBuffers(renderPass, 0, &spriteDataBuffer, 1);
//			SDL_BindGPUFragmentSamplers(
//				renderPass,
//				0,
//				&(SDL_GPUTextureSamplerBinding){
//					.texture = Texture,
//					.sampler = Sampler
//				},
//				1
//			);
			
			constexpr uint32_t kVerticesPerSprite = 6;
			
			SDL_DrawGPUPrimitives(renderPass, kNumSprites * kVerticesPerSprite, 1, 0, 0);
			SDL_EndGPURenderPass(renderPass);
		}
	}
	
	SDL_SubmitGPUCommandBuffer(cmdbuf);
	
	return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* /*appstate*/, SDL_AppResult /*result*/) {
	SDL_ReleaseGPUTransferBuffer(device, transferBuffer);
	SDL_ReleaseGPUTransferBuffer(device, spriteDataTransferBuffer);
	SDL_ReleaseGPUBuffer(device, vertexBuffer);
	SDL_ReleaseGPUBuffer(device, spriteDataBuffer);
	SDL_ReleaseGPUGraphicsPipeline(device, spritePipeline);
	SDL_ReleaseGPUGraphicsPipeline(device, pipeline);
	SDL_ReleaseWindowFromGPUDevice(device, window);
	SDL_DestroyGPUDevice(device);
	SDL_DestroyWindow(window);
}

} // extern "C"
