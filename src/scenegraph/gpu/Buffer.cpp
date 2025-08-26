#include <scenegraph/gpu/Buffer.h>

#include <SDL3/SDL.h>

constexpr SDL_GPUBufferUsageFlags GetBufferUsageFlags(BufferUsage usage) noexcept {
	switch (usage) {
	case BufferUsage::Invalid: return 0;
	case BufferUsage::Vertex: return SDL_GPU_BUFFERUSAGE_VERTEX;
	case BufferUsage::Index: return SDL_GPU_BUFFERUSAGE_INDEX;
	case BufferUsage::Indirect: return SDL_GPU_BUFFERUSAGE_INDIRECT;
	case BufferUsage::GraphicsStorageRead: return SDL_GPU_BUFFERUSAGE_GRAPHICS_STORAGE_READ;
	case BufferUsage::ComputeStorageRead: return SDL_GPU_BUFFERUSAGE_GRAPHICS_STORAGE_READ;
	case BufferUsage::ComputeStorageWrite: return SDL_GPU_BUFFERUSAGE_COMPUTE_STORAGE_WRITE;
	}
}

void Buffer::Create(BufferUsage usage, uint32_t size) noexcept {
	SDL_assert(_handle == nullptr);
	if (_handle) {
		Destroy();
	}
	
	if (auto device = static_cast<SDL_GPUDevice*>(_device)) {
		SDL_GPUBufferCreateInfo createInfo = {
			.usage = GetBufferUsageFlags(usage),
			.size = size
		};
		_handle = SDL_CreateGPUBuffer(device, &createInfo);
		SDL_assert(_handle != nullptr);
	}
}

void Buffer::Destroy() noexcept {
	if (auto buffer = static_cast<SDL_GPUBuffer*>(_handle)) {
		SDL_ReleaseGPUBuffer(static_cast<SDL_GPUDevice*>(_device), buffer);
		_handle = nullptr;
	}
}
