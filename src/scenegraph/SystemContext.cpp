#include <scenegraph/SystemContext.h>
#include <scenegraph/ApplicationContext.h>

#include <cassert>

#include <SDL3/SDL.h>

SystemContext::~SystemContext() {
	auto device = static_cast<SDL_GPUDevice*>(_device);
	auto window = static_cast<SDL_Window*>(_window);
	
	if (device && window) { SDL_ReleaseWindowFromGPUDevice(device, window); }
	if (window) { SDL_DestroyWindow(window); }
	if (device) { SDL_DestroyGPUDevice(device); }
}

bool SystemContext::Initialize() {
	SDL_assert(!_initialized);
	if (_initialized) {
		return true;
	}
	
	SDL_Init(SDL_INIT_VIDEO);
	
	bool debugMode = false;
#ifndef NDEBUG
	debugMode = true;
#endif
	
	auto device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_METALLIB, debugMode, nullptr);
	if (!device) {
		SDL_Log("GPUCreateDevice failed");
		return false;
	}
	
	_device = device;
	
	auto appCtx = GetApplicationContext();
	
	auto window = SDL_CreateWindow(appCtx->GetApplicationName(), 1280, 720, SDL_WINDOW_RESIZABLE /*| SDL_WINDOW_HIGH_PIXEL_DENSITY*/);
	if (!window) {
		SDL_Log("CreateWindow failed: %s", SDL_GetError());
		return false;
	}
	
	_window = window;
	
	if (!SDL_ClaimWindowForGPUDevice(device, window)) {
		SDL_Log("GPUClaimWindow failed");
		return false;
	}
	
	_initialized = true;
	
	return true;
}

void SystemContext::Finalize() {
	_instance.reset();
}

Buffer SystemContext:: CreateBuffer() noexcept {
	return Buffer{_device};
}

TransferBuffer SystemContext::CreateTransferBuffer() noexcept {
	return TransferBuffer{_device};
}
