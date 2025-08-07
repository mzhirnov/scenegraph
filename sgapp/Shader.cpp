#include "Shader.h"

SDL_GPUShader* LoadShader(
	SDL_GPUDevice* device,
	const unsigned char* code,
	std::size_t codeSize,
	SDL_GPUShaderStage stage,
	Uint32 samplerCount,
	Uint32 uniformBufferCount,
	Uint32 storageBufferCount,
	Uint32 storageTextureCount
) {
	SDL_GPUShaderCreateInfo shaderInfo = {
		.code = code,
		.code_size = codeSize,
		.entrypoint = "main0",
		.format = SDL_GPU_SHADERFORMAT_METALLIB,
		.stage = stage,
		.num_samplers = samplerCount,
		.num_uniform_buffers = uniformBufferCount,
		.num_storage_buffers = storageBufferCount,
		.num_storage_textures = storageTextureCount
	};
	
	SDL_GPUShader* shader = SDL_CreateGPUShader(device, &shaderInfo);
	return shader;
}
