#pragma once

#include <scenegraph/gpu/Resource.h>

#include <cstdint>

enum class BufferUsage {
	Invalid,
	Vertex = 1 << 0,
	Index = 1 << 1,
	Indirect = 1 << 2,
	GraphicsStorageRead = 1 << 3,
	ComputeStorageRead = 1 << 4,
	ComputeStorageWrite = 1 << 5
};

///
/// Buffer
///
class Buffer : public Resource {
public:
	explicit Buffer(void* device) : Resource(device)
	{
	}
	
	~Buffer() { Destroy(); }
	
	void Create(BufferUsage usage, uint32_t size) noexcept;
	void Destroy() noexcept;
};
