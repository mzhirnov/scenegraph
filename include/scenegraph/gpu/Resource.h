#pragma once

#include <scenegraph/utils/NonCopyable.h>

///
/// Resource
///
class Resource : public NonCopyable {
public:
	explicit Resource(void* device)
		: _device(device)
	{
	}
	
	Resource(Resource&& rhs) noexcept
		: _device(rhs._device)
		, _handle(rhs._handle)
	{
		rhs._handle = {};
	}
	
	Resource& operator=(Resource&& rhs) noexcept {
		if (&rhs != this) {
			_device = rhs._device;
			_handle = rhs._handle;
			
			rhs._handle = {};
		}
		return *this;
	}
	
	void* Device() const noexcept { return _device; }
	
	void* Handle() const noexcept { return _handle; }
	
	bool Valid() const noexcept { return _handle != nullptr; }

protected:
	void* _device = nullptr;
	
	void* _handle = nullptr;
};
