#pragma once

#include <cstddef>

#include <SDL3/SDL.h>

SDL_GPUShader* LoadShader(
	SDL_GPUDevice* device,
	const unsigned char* code,
	std::size_t codeSize,
	SDL_GPUShaderStage stage,
	Uint32 samplerCount = 0,
	Uint32 uniformBufferCount = 0,
	Uint32 storageBufferCount = 0,
	Uint32 storageTextureCount = 0
);
