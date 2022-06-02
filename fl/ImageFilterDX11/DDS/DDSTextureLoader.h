//--------------------------------------------------------------------------------------
// File: DDSTextureLoader.h
//
// Functions for loading a DDS texture without using D3DX
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#include <d3d9.h>
#include <d3d11.h>

struct SDDSInfo
{
	UINT iWidth;
	UINT iHeight;
	UINT iMipCount;
};

HRESULT CreateDDSTextureFromFile( ID3D11Device* pDev, const char* szFileName, ID3D11ShaderResourceView** ppSRV, bool sRGB = false );

HRESULT CreateDDSTextureFromBuffer(ID3D11Device* pDev, BYTE* pData, int nDataSize, ID3D11ShaderResourceView** ppSRV, SDDSInfo *ddsInfo = NULL, bool sRGB=false);