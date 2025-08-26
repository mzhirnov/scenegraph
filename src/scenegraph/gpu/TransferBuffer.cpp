#include <scenegraph/gpu/TransferBuffer.h>

#include <SDL3/SDL.h>

constexpr SDL_GPUTransferBufferUsage GetTransferBufferUsage(TransferBufferUsage usage) noexcept {
	switch (usage) {
	case TransferBufferUsage::Upload: return SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
	case TransferBufferUsage::Download: return SDL_GPU_TRANSFERBUFFERUSAGE_DOWNLOAD;
	}
}

void TransferBuffer::Create(TransferBufferUsage usage, uint32_t size) noexcept {
	SDL_assert(_handle == nullptr);
	if (_handle) {
		Destroy();
	}
	
	if (auto device = static_cast<SDL_GPUDevice*>(_device)) {
		SDL_GPUTransferBufferCreateInfo createInfo = {
			.usage = GetTransferBufferUsage(usage),
			.size = size
		};
		_handle = SDL_CreateGPUTransferBuffer(device, &createInfo);
		SDL_assert(_handle != nullptr);
	}
}

void TransferBuffer::Destroy() noexcept {
	if (auto buffer = static_cast<SDL_GPUTransferBuffer*>(_handle)) {
		SDL_ReleaseGPUTransferBuffer(static_cast<SDL_GPUDevice*>(_device), buffer);
		_handle = nullptr;
	}
}

void* TransferBuffer::Lock(BufferLockMode mode) noexcept {
	return SDL_MapGPUTransferBuffer(
		static_cast<SDL_GPUDevice*>(_device),
		static_cast<SDL_GPUTransferBuffer*>(_handle),
		mode == BufferLockMode::Dynamic
	);
}

void TransferBuffer::Unlock() noexcept {
	SDL_UnmapGPUTransferBuffer(static_cast<SDL_GPUDevice*>(_device), static_cast<SDL_GPUTransferBuffer*>(_handle));
}
