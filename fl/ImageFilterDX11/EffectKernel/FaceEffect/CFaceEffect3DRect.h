#pragma once

#include "EffectKernel/CEffectPart.h"
#include "Toolbox/EffectModel.hpp"
#include "Toolbox/FSObject.h"
#include "Toolbox/FacialObject.h"
#include "BaseDefine/Vectors.h"
#include "Algorithm/Face2DMesh/MTFace2DInterFace.h"

struct FaceBoundingBox
{
	Vector3 maxPoint;
	Vector3 minPoint;
	Vector3 centerPoint;
};

struct MyRect3D
{
	Vector3 LT;
	Vector3 RT;
	Vector3 LB;
	Vector3 RB;
};

struct HeadRectVertex  
{
	Vector3 Vertex;
	Vector3 Normal;
	Vector2 UV;
};

class CFaceEffect3DRect : public CEffectPart
{
public:
	CFaceEffect3DRect();
	~CFaceEffect3DRect();

	virtual void Release();
	virtual void* Clone();
	virtual bool ReadConfig(XMLNode& childNode, HZIP hZip = 0, char *pFilePath = NULL);
	virtual bool Prepare();
	virtual void Render(BaseRenderParam &RenderParam);


private:

	void GeneratePoint(Vector2 *pFacePoint);
	FaceBoundingBox m_BoundingBox;
	std::vector<HeadRectVertex>m_HeadRect;
	std::vector<HeadRectVertex>m_HeadRectNormal;
	MyRect3D Front;
	MyRect3D Back;

	ID3D11Buffer*       m_pConstantBuffer;  
	ID3D11Buffer *m_IndexBuffer;
	ID3D11Buffer *m_VerticeBuffer;
	mt3dface::MTFace2DInterFace *m_2DInterFace;

	ID3D11DepthStencilState *m_pDepthStateEnable;
	ID3D11DepthStencilState *m_pDepthStateDisable;

	DX11Texture *m_material;

	Vector3 m_ScaleBottom = Vector3(1.0f, 1.0f, 1.0f);
};
