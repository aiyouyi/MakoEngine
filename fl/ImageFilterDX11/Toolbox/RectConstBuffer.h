#pragma once
#include <xnamath.h>
#include "BaseDefine/Vectors.h"

struct BaseRectVertex
{
	XMFLOAT3 Pos;//λ��  
	XMFLOAT2 TexCoord;//��ɫ  
};
struct RectConstantBuffer
{
	XMMATRIX mWVP; //��Ͼ���
};
struct RectConstantBufferMask
{
	XMMATRIX mWVP; //��Ͼ���
	XMFLOAT4 mClip;//�����ü�����
	Vector2 texSize;
};