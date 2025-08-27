#include <scenegraph/ApplicationContext.h>
#include <scenegraph/SystemContext.h>

#include <scenegraph/utils/ScopeGuard.h>
#include <scenegraph/math/Matrix4.h>
#include <scenegraph/math/Matrix32.h>
#include <scenegraph/render/Vertex.h>
#include <scenegraph/imaging/Color.h>

#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "Imaging.h"

static SDL_GPUShader* LoadShader(SDL_GPUDevice* device_, SDL_GPUShaderCreateInfo* createInfo, const char* filename) {
	size_t size;
	auto data = SDL_LoadFile(filename, &size);
	if (!data) {
		return nullptr;
	}
	
	ON_SCOPE_EXIT(&data) { SDL_free(data); };
	
	createInfo->code = static_cast<Uint8*>(data);
	createInfo->code_size = size;
	
	return SDL_CreateGPUShader(device_, createInfo);
}

static SDL_Window* window;
static SDL_GPUDevice* device;
static SDL_GPUGraphicsPipeline* pipeline;
static SDL_GPUGraphicsPipeline* spritePipeline;
static SDL_GPUBuffer* vertexBuffer;
static SDL_GPUBuffer* spriteDataBuffer;
static SDL_GPUTransferBuffer* transferBuffer;
static SDL_GPUTransferBuffer* spriteDataTransferBuffer;
static SDL_GPUSampler* sampler;
static SDL_GPUTexture* texture;

struct SpriteInstance {
	Matrix32 transform;
	float pivotX;
	float pivotY;
	FloatColor color;
	float texU, texV, texW, texH;
};

using PositionColorVertex = Vertex<Position3<>, Color<>>;

template <typename T>
constexpr SDL_GPUVertexElementFormat VertexElementFormat = SDL_GPU_VERTEXELEMENTFORMAT_INVALID;

template <std::size_t Stage> constexpr SDL_GPUVertexElementFormat VertexElementFormat<Position3<Stage>> = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
template <std::size_t Stage> constexpr SDL_GPUVertexElementFormat VertexElementFormat<Color<Stage>> = SDL_GPU_VERTEXELEMENTFORMAT_UBYTE4_NORM;

static SDL_AppResult ExampleInitialize() {
	// Create the shaders
	SDL_GPUShaderCreateInfo vertexShaderCreateInfo = {
		.format = SDL_GPU_SHADERFORMAT_METALLIB,
		.stage = SDL_GPU_SHADERSTAGE_VERTEX,
		.num_uniform_buffers = 1
	};
	SDL_GPUShader* vertexShader = LoadShader(device, &vertexShaderCreateInfo, "../assets/shaders/PositionColorTransform.vert.air");
	if (!vertexShader) {
		SDL_Log("Failed to create vertex shader!");
		return SDL_APP_FAILURE;
	}
	
	SDL_GPUShaderCreateInfo fragmentShaderCreateInfo = {
		.format = SDL_GPU_SHADERFORMAT_METALLIB,
		.stage = SDL_GPU_SHADERSTAGE_FRAGMENT
	};
	SDL_GPUShader* fragmentShader = LoadShader(device, &fragmentShaderCreateInfo, "../assets/shaders/SolidColor.frag.air");
	if (!fragmentShader) {
		SDL_Log("Failed to create fragment shader!");
		return SDL_APP_FAILURE;
	}
	
	SDL_GPUShaderCreateInfo spriteVertexShaderCreateInfo = {
		.format = SDL_GPU_SHADERFORMAT_METALLIB,
		.stage = SDL_GPU_SHADERSTAGE_VERTEX,
		.num_uniform_buffers = 1,
		.num_storage_buffers = 1
	};
	SDL_GPUShader* spriteVertexShader = LoadShader(device, &spriteVertexShaderCreateInfo, "../assets/shaders/PullSpriteBatch.vert.air");
	if (!spriteVertexShader) {
		SDL_Log("Failed to create vertex shader!");
		return SDL_APP_FAILURE;
	}

	SDL_GPUShaderCreateInfo spriteFragmentShaderCreateInfo = {
		.format = SDL_GPU_SHADERFORMAT_METALLIB,
		.stage = SDL_GPU_SHADERSTAGE_FRAGMENT,
		.num_samplers = 1
	};
	SDL_GPUShader* spriteFragmentShader = LoadShader(device, &spriteFragmentShaderCreateInfo, "../assets/shaders/TexturedQuadColor.frag.air");
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
			.dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA
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
		SDL_Log("Failed to create line pipeline");
        return SDL_APP_FAILURE;
    }
	
	SDL_GPUGraphicsPipelineCreateInfo spritePipelineCreateInfo = {
		.target_info = {
			.num_color_targets = SDL_arraysize(targetDescriptions),
			.color_target_descriptions = targetDescriptions,
		},
		//.rasterizer_state
		//.multisample_.depth_stencil_state.enable_depth_test
		.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
		.vertex_shader = spriteVertexShader,
		.fragment_shader = spriteFragmentShader
	};
	
	spritePipeline = SDL_CreateGPUGraphicsPipeline(device, &spritePipelineCreateInfo);
	if (!spritePipeline) {
		SDL_Log("Failed to create sprite pipeline");
        return SDL_APP_FAILURE;
    }
	
	// Clean up shader resources
	SDL_ReleaseGPUShader(device, vertexShader);
	SDL_ReleaseGPUShader(device, fragmentShader);
	SDL_ReleaseGPUShader(device, spriteVertexShader);
	SDL_ReleaseGPUShader(device, spriteFragmentShader);
	
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
	
	SDL_GPUSamplerCreateInfo samplerCreateInfo = {
		.min_filter = SDL_GPU_FILTER_LINEAR,
		.mag_filter = SDL_GPU_FILTER_LINEAR,
		.mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_LINEAR,
		.address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
		.address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
		.address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE
	};
	sampler = SDL_CreateGPUSampler(device, &samplerCreateInfo);
	
	SDL_GPUTextureQOILoader loader(device);
	texture = loader.LoadFromFile("../assets/textures/checkerboard.qoi");
	if (!texture) {
		SDL_Log("Failed to load texture");
		return SDL_APP_FAILURE;
	}
	SDL_Log("Loaded %dx%dx%d texture", loader.Width(), loader.Height(), loader.Channels());
	
	return SDL_APP_CONTINUE;
}

static SDL_AppResult ExampleIterate() {
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
				.store_op = SDL_GPU_STOREOP_STORE
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
				auto angle = SDL_PI_F / (kNumSprites - 1) * i;
				dataPtr[i] = {
					.transform = Matrix32MakeWithTransform2D(
						{ .sx = 32, .sy = 32, .shearX = 0.5f, .rad = angle, .tx = 100 + i * 50.0f, .ty = 100 }),
					.pivotX = 0.5f,
					.pivotY = 0.5f,
					.color = { (255 - 10 * i) / 255.0f, (255 - 5 * i) / 255.0f, 1.0f, 1.0f },
					.texU = 0.5f / 256,
					.texV = 0.5f / 256,
					.texW = 1 - 0.5f / 256,
					.texH = 1 - 0.5f / 256
				};
			}
			dataPtr[20] = {
				.transform = Matrix32MakeWithTransform2D({ .sx = 128, .sy = 128, .tx = 100, .ty = 100 }),
				.pivotX = 0.5f,
				.pivotY = 0.5f,
				.color = FloatColorMakeWhite(),
				.texU = 0.5f / 256,
				.texV = 0.5f / 256,
				.texW = 1 - 0.5f / 256,
				.texH = 1 - 0.5f / 256
			};
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
			
			SDL_GPUTextureSamplerBinding textureSamplerBindings[1] = {{
				.texture = texture,
				.sampler = sampler
			}};
			SDL_BindGPUFragmentSamplers(renderPass, 0, textureSamplerBindings, SDL_arraysize(textureSamplerBindings));
			
			constexpr uint32_t kVerticesPerSprite = 6;
			
			SDL_DrawGPUPrimitives(renderPass, kNumSprites * kVerticesPerSprite, 1, 0, 0);
			SDL_EndGPURenderPass(renderPass);
		}
	}
	
	SDL_SubmitGPUCommandBuffer(cmdbuf);
	
	return SDL_APP_CONTINUE;
}

static void ExampleFinalize() {
	SDL_ReleaseGPUTexture(device, texture);
	SDL_ReleaseGPUSampler(device, sampler);
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

extern "C" {

SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[]) {
	auto appCtx = GetApplicationContext();
	SDL_assert(appCtx != nullptr);
	if (!appCtx) {
		return SDL_APP_FAILURE;
	}
	
	*appstate = appCtx;
	
	SDL_Init(SDL_INIT_VIDEO);
	
	window = SDL_CreateWindow(appCtx->GetApplicationName(), 1280, 720, SDL_WINDOW_RESIZABLE /*| SDL_WINDOW_HIGH_PIXEL_DENSITY*/);
	if (!window) {
		SDL_Log("CreateWindow failed: %s", SDL_GetError());
		return SDL_APP_FAILURE;
	}
	
	bool debugMode = false;
#ifndef NDEBUG
	debugMode = true;
#endif
	
	device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_METALLIB, debugMode, nullptr);
	if (!device) {
		SDL_Log("GPUCreateDevice failed");
		return SDL_APP_FAILURE;
	}
	
	if (!SDL_ClaimWindowForGPUDevice(device, window)) {
		SDL_Log("GPUClaimWindow failed");
		return SDL_APP_FAILURE;
	}
	
	return appCtx->Initialize(argc, argv) ? /*SDL_APP_CONTINUE*/ ExampleInitialize() : SDL_APP_SUCCESS;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
	auto appCtx = static_cast<ApplicationContext*>(appstate);
	SDL_assert(appCtx != nullptr);
	if (!appCtx) {
		return SDL_APP_FAILURE;
	}
	
	switch (event->type) {
	case SDL_EVENT_QUIT:
		return SDL_APP_SUCCESS;
	}
	
	return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* /*appstate*/) {
	return /*SDL_APP_CONTINUE*/ ExampleIterate();
}

void SDL_AppQuit(void* appstate, SDL_AppResult /*result*/) {
	ExampleFinalize();
	
	auto appCtx = static_cast<ApplicationContext*>(appstate);
	SDL_assert(appCtx != nullptr);
	appCtx ? appCtx->Finalize() : (void)0;
}

} // extern "C"
