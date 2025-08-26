#pragma once

#include <scenegraph/gpu/Buffer.h>
#include <scenegraph/gpu/TransferBuffer.h>

#include <memory>

///
/// System context
///
class SystemContext {
public:
	SystemContext() = default;
	
	~SystemContext();
	
	bool Initialize();
	void Finalize();
	
	Buffer CreateBuffer() noexcept;
	TransferBuffer CreateTransferBuffer() noexcept;
	
public:
	static SystemContext* GetInstance() noexcept {
		if (!_instance) {
			_instance = std::make_unique<SystemContext>();
		}
		return _instance.get();
	}
	
private:
	bool _initialized = false;
	void* _device = nullptr;
	void* _window = nullptr;
	
	inline static std::unique_ptr<SystemContext> _instance;
};

inline SystemContext* GetSystemContext() noexcept { return SystemContext::GetInstance(); }
