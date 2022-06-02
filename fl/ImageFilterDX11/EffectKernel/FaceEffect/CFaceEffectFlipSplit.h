#pragma once
#include "Toolbox/DXUtils/DXUtils.h"
#include "EffectKernel/CEffectPart.h"
#include "Toolbox/Render/CC3DShaderDef.h"
#include "Algorithm/Face2DMesh/MTFace2DInterFace.h"


class CFaceEffectFlipSplit : public CEffectPart
{
public:

	struct Vertex
	{
		XMFLOAT3 Pos;
		XMFLOAT2 UV;
	};


	CFaceEffectFlipSplit();
	virtual ~CFaceEffectFlipSplit();
	virtual void Release();
	virtual void* Clone();
	virtual bool ReadConfig(XMLNode& childNode, HZIP hZip = 0, char* pFilePath = NULL);
	virtual bool Prepare();
	virtual void Render(BaseRenderParam& RenderParam);

	Vector4 mControlParam = Vector4(0, 0, 0, 0);

private:
	std::shared_ptr<CC3DVertexBuffer> mVertexBuffer;
	std::shared_ptr<CC3DIndexBuffer> mIndexBuffer;

	ID3D11Buffer* m_pConstantBuffer;
};

