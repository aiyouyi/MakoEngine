#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H
#include <Runtime/Core/Public/CoreMinimal.h>
#include <vector>

class CCACTORLIB_API AudioManager
{
public:
	AudioManager();
	~AudioManager();

	bool InitDevice(const wchar_t* deviceName,int& outDevice);
	void Play(uint32_t handle, bool restart);
	void Stop(uint32_t handle);
	void SetVol(uint32_t handle,float vol);
	uint32_t CreateStreamFromFile(const wchar_t* fileName, uint64_t offset, uint64_t length);
	void ChangeIsLoop(uint32_t handle, bool shouldLoop);
	void FreeStream(uint32_t handle);
	bool IsStreamPlaying(uint32_t handle);

	std::vector<FString> mDevList;
private:
	void InitDeviceList();
	void OutputBassError();

	
};
#endif