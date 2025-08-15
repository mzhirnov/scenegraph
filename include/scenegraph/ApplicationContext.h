#pragma once

///
/// Application context
///
class ApplicationContext {
public:
	virtual bool Initialize(int argc, char* argv[]) = 0;
	
	virtual bool Iterate() = 0;
	
	virtual void Finalize() = 0;
};

extern ApplicationContext* GetApplicationContext();
