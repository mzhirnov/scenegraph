#include "Imaging.h"

#include <SDL3/SDL.h>

#define QOI_OP_INDEX  0x00 /* 00xxxxxx */
#define QOI_OP_DIFF   0x40 /* 01xxxxxx */
#define QOI_OP_LUMA   0x80 /* 10xxxxxx */
#define QOI_OP_RUN    0xc0 /* 11xxxxxx */
#define QOI_OP_RGB    0xfe /* 11111110 */
#define QOI_OP_RGBA   0xff /* 11111111 */

#define QOI_MASK_2    0xc0 /* 11000000 */

#define QOI_COLOR_HASH(C) (C.rgba.r*3 + C.rgba.g*5 + C.rgba.b*7 + C.rgba.a*11)
#define QOI_MAGIC \
	(((unsigned int)'q') << 24 | ((unsigned int)'o') << 16 | \
	 ((unsigned int)'i') <<  8 | ((unsigned int)'f'))
#define QOI_HEADER_SIZE 14

#define QOI_SRGB   0
#define QOI_LINEAR 1

#define QOI_PIXELS_MAX ((unsigned int)400000000)

union qoi_rgba_t {
	struct { uint8_t r, g, b, a; } rgba;
	uint32_t v;
};

constexpr uint8_t qoi_padding[8] = {0,0,0,0,0,0,0,1};

struct qoi_desc {
	uint32_t width;
	uint32_t height;
	uint8_t channels;
	uint8_t colorspace;
};

static inline uint32_t qoi_read_32(const uint8_t* bytes, uint32_t* p) {
	uint32_t a = bytes[(*p)++];
	uint32_t b = bytes[(*p)++];
	uint32_t c = bytes[(*p)++];
	uint32_t d = bytes[(*p)++];
	return a << 24 | b << 16 | c << 8 | d;
}

bool QOILoader::BeginLoad(const uint8_t* bytes, uint32_t size, uint32_t* width, uint32_t* height, uint8_t* channels) noexcept {
	_bytes = bytes;
	_size = size;
	_p = 0;
	
	uint32_t magic = qoi_read_32(bytes, &_p);
	*width = qoi_read_32(bytes, &_p);
	*height = qoi_read_32(bytes, &_p);
	*channels = bytes[_p++];
	uint8_t colorspace = bytes[_p++];
	
	if (!*width || !*height || *channels < 3 || *channels > 4 || colorspace > 1 || magic != QOI_MAGIC || *height >= QOI_PIXELS_MAX / *width) {
		return false;
	}
	
	return true;
}

bool QOILoader::EndLoad(uint8_t* pixels, uint32_t size, uint32_t channels) noexcept {
	if (!pixels || channels < 3 || channels > 4) {
		return false;
	}
	
	qoi_rgba_t px = { .rgba = { .a = 255 } };
	qoi_rgba_t index[64] {};

	uint32_t chunks_len = static_cast<uint32_t>(static_cast<uint32_t>(_size) - sizeof(qoi_padding));
	uint32_t run = 0;
	
	for (uint32_t px_pos = 0; px_pos < size; px_pos += channels) {
		if (run > 0) {
			run--;
		}
		else if (_p < chunks_len) {
			int b1 = _bytes[_p++];

			if (b1 == QOI_OP_RGB) {
				px.rgba.r = _bytes[_p++];
				px.rgba.g = _bytes[_p++];
				px.rgba.b = _bytes[_p++];
			}
			else if (b1 == QOI_OP_RGBA) {
				px.rgba.r = _bytes[_p++];
				px.rgba.g = _bytes[_p++];
				px.rgba.b = _bytes[_p++];
				px.rgba.a = _bytes[_p++];
			}
			else if ((b1 & QOI_MASK_2) == QOI_OP_INDEX) {
				px = index[b1];
			}
			else if ((b1 & QOI_MASK_2) == QOI_OP_DIFF) {
				px.rgba.r += ((b1 >> 4) & 0x03) - 2;
				px.rgba.g += ((b1 >> 2) & 0x03) - 2;
				px.rgba.b += ( b1       & 0x03) - 2;
			}
			else if ((b1 & QOI_MASK_2) == QOI_OP_LUMA) {
				int b2 = _bytes[_p++];
				int vg = (b1 & 0x3f) - 32;
				px.rgba.r += vg - 8 + ((b2 >> 4) & 0x0f);
				px.rgba.g += vg;
				px.rgba.b += vg - 8 +  (b2       & 0x0f);
			}
			else if ((b1 & QOI_MASK_2) == QOI_OP_RUN) {
				run = (b1 & 0x3f);
			}

			index[QOI_COLOR_HASH(px) & (64 - 1)] = px;
		}

		*pixels++ = px.rgba.r;
		*pixels++ = px.rgba.g;
		*pixels++ = px.rgba.b;
		
		if (channels == 4) {
			*pixels++ = px.rgba.a;
		}
	}
		
	return true;
}

SDL_GPUTexture* SDL_GPUTextureQOILoader::LoadFromFile(const char* filename) noexcept {
	size_t size;
	auto data = SDL_LoadFile(filename, &size);
	if (!data) {
		return nullptr;
	}
	
	uint32_t width, height;
	uint8_t channels;
	
	if (BeginLoad(static_cast<uint8_t*>(data), static_cast<uint32_t>(size), &width, &height, &channels)) {
		channels = 4;
		
		SDL_GPUTransferBufferCreateInfo transferBufferCreateInfo = {
			.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
			.size = width * height * channels
		};
		auto transferBuffer = SDL_CreateGPUTransferBuffer(_device, &transferBufferCreateInfo);
		
		auto pixels = static_cast<uint8_t*>(SDL_MapGPUTransferBuffer(_device, transferBuffer, false));
		
		auto ok = EndLoad(pixels, width * height * channels, channels);
		
		SDL_UnmapGPUTransferBuffer(_device, transferBuffer);
		
		SDL_free(data);
		
		if (ok) {
			SDL_GPUTextureCreateInfo textureCreateInfo = {
				.type = SDL_GPU_TEXTURETYPE_2D,
				.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM,
				.width = width,
				.height = height,
				.layer_count_or_depth = 1,
				.num_levels = 1,
				.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER
			};
			
			auto texture = SDL_CreateGPUTexture(_device, &textureCreateInfo);
			if (texture) {
				SDL_GPUCommandBuffer* cmdbuf = SDL_AcquireGPUCommandBuffer(_device);
				SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(cmdbuf);
				
				SDL_GPUTextureTransferInfo textureTransferInfo = {
					.transfer_buffer = transferBuffer,
					.offset = 0,
				};
				SDL_GPUTextureRegion region = {
					.texture = texture,
					.w = width,
					.h = height,
					.d = 1
				};
				SDL_UploadToGPUTexture(
					copyPass,
					&textureTransferInfo,
					&region,
					false
				);
				
				SDL_EndGPUCopyPass(copyPass);
				SDL_SubmitGPUCommandBuffer(cmdbuf);
				
				SDL_ReleaseGPUTransferBuffer(_device, transferBuffer);
				
				return texture;
			}
			
			return nullptr;
		}
		
		return nullptr;
	}
	
	return nullptr;
}
