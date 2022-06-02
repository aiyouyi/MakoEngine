#pragma once
#include "Toolbox/Render/ConstantBuffer.h"
#include "common.h"

class DX11ConstantBuffer : public CC3DConstantBuffer
{
public:
	DX11ConstantBuffer();
	virtual ~DX11ConstantBuffer();

	virtual void CreateBuffer(int32_t ByteSize);

public:
	ID3D11Buffer* PtrConstantBuffer = nullptr;
};