#pragma once

#include <cstddef>

#include <SDL3/SDL.h>

SDL_GPUShader* LoadShader(
	SDL_GPUDevice* device,
	const unsigned char* code,
	std::size_t codeSize,
	SDL_GPUShaderStage stage,
	Uint32 samplerCount,
	Uint32 uniformBufferCount,
	Uint32 storageBufferCount,
	Uint32 storageTextureCount
);
