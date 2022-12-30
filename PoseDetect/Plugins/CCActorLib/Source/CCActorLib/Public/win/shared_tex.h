#pragma once
#include <Runtime/Core/Public/CoreMinimal.h>
#include "core/inc.h"

struct D3D11_TEXTURE2D_DESC;
struct D3D12_RESOURCE_DESC;
struct ID3D12Resource;
struct SharedTexP;

class CCACTORLIB_API SharedTex
{
public:
	enum API : uint8_t 
	{
		DirectX_11,
		DirectX_12,
	};
public:
	SharedTex();
	~SharedTex();

	bool init(void *nativeRes);
	void doSharedCopy(void *nativeRes);
	std::string getName() { return mName; }
	bool isValid();
	uint64_t getSharedHandle();
	void Dump(void* nativeRes);
private:
	bool initDX11(void *nativeRes);
	bool initDX12(void *nativeRes);
	bool createSharedTexture(core::vec2i size, const D3D11_TEXTURE2D_DESC& desc);
	bool createSharedTexture(ID3D12Resource* d3d12Res);
	void doDX11SharedCopy(void *nativeRes);
	void doDX12SharedCopy(void *nativeRes);
	bool initD3D11On12();

private:
	API mAPI;
	std::string mName;
	SharedTexP* mData;
};