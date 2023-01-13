#pragma once
#include <xnamath.h>
#include "BaseDefine/Vectors.h"

struct BaseRectVertex
{
	XMFLOAT3 Pos;//位置  
	XMFLOAT2 TexCoord;//颜色  
};
struct RectConstantBuffer
{
	XMMATRIX mWVP; //混合矩阵
};
struct RectConstantBufferMask
{
	XMMATRIX mWVP; //混合矩阵
	XMFLOAT4 mClip;//背景裁剪区域
	Vector2 texSize;
};