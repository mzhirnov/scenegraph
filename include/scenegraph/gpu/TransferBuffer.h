#pragma once

#include <scenegraph/gpu/Resource.h>

#include <cstdint>

enum class TransferBufferUsage {
	Upload,
	Download
};

enum class BufferLockMode {
	Static,
	Dynamic
};

///
/// Transfer buffer
///
class TransferBuffer : public Resource {
public:
	explicit TransferBuffer(void* device) : Resource(device)
	{
	}
	
	~TransferBuffer() { Destroy(); }
	
	void Create(TransferBufferUsage usage, uint32_t size) noexcept;
	void Destroy() noexcept;
	
	void* Lock(BufferLockMode mode = BufferLockMode::Static) noexcept;
	void Unlock() noexcept;
};
