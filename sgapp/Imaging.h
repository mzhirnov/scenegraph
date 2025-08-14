#pragma once

#include <cstdint>

struct SDL_GPUDevice;
struct SDL_GPUTexture;

class QOILoader {
public:
	bool BeginLoad(const uint8_t* bytes, uint32_t size, uint32_t* width, uint32_t* height, uint8_t* channels) noexcept;
	bool EndLoad(uint8_t* pixels, uint32_t size, uint32_t channels) noexcept;
	
private:
	const uint8_t* _bytes = nullptr;
	uint32_t _size = 0;
	uint32_t _p = 0;
};

class SDL_GPUTextureQOILoader final : public QOILoader {
public:
	explicit SDL_GPUTextureQOILoader(SDL_GPUDevice* device)
		: _device(device)
	{
	}
	
	SDL_GPUTexture* LoadFromFile(const char* filename) noexcept;
	
	uint32_t Width() const noexcept { return _width; }
	uint32_t Height() const noexcept { return _height; }
	uint8_t Channels() const noexcept { return _channels; }
	
private:
	SDL_GPUDevice* _device = nullptr;
	uint32_t _width = 0;
	uint32_t _height = 0;
	uint8_t _channels = 0;
};
