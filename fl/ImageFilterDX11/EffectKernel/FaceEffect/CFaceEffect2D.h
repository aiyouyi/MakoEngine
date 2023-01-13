#pragma once
#include "Toolbox/DXUtils/DXUtils.h"
#include "EffectKernel/CEffectPart.h"
#include "Algorithm/Face2DMesh/MTFace2DInterFace.h"
#include "Toolbox/Drawable.hpp"
#include "EffectKernel/DXBasicSample.h"

enum MixType
{
	MIX_NORMAL = 0,
	MIX_SOFTLIGHT,
	MIX_MULTIPLY
}; 
//struct FaceEffectInfo
//{
//	//DX11Texture *m_material;
//	//Drawable *m_drawable;
//	//DX11Texture *m_mask;
//	
//};

class CFaceEffect2D : public CEffectPart, protected DXBaicSample
{
public:
	CFaceEffect2D();
	virtual ~CFaceEffect2D();
	virtual void Release();
	virtual void* Clone();
	virtual bool ReadConfig(XMLNode& childNode, HZIP hZip = 0,char *pFilePath=NULL);
	virtual bool ReadConfig(XMLNode& childNode, const std::string &path);
	virtual bool Prepare();
	virtual void Render(BaseRenderParam &RenderParam);

	Vector2 m_SrcRect[4];
private:
	void MergeVertex(float *pVertex, float *pUV,float *pMaskUV, int nVertex);
	void ReadConfig(XMLNode& childNode, HZIP hZip = 0, char *pFilePath = NULL, const std::string &path = "");
	long long GetImageID(HZIP hZip, const std::string & path, const char *szDrawableName);
	bool WriteConfig(std::string &tempPath, XMLNode &root, HZIP dst, HZIP src);
	//FaceEffectInfo *m_FaceEffectInfo;
	MixType m_MixType = MIX_NORMAL;
	long long m_MaskId = 0;

	ID3D11Buffer *m_VerticeBuffer[MAX_SUPPORT_PEOPLE];
	ID3D11Buffer *m_IndexBuffer;
	ID3D11Buffer*       m_pConstantBuffer;

	mt3dface::MTFace2DInterFace *m_2DInterFace;
	Vector2 m_SrcPoint[3];
	Vector2 m_DstPoint[3] = { Vector2(0.0, 0.0), Vector2(1.0, 0.0), Vector2(1.0, 1.0)};

	float *m_pMergeVertex;

	bool m_AffineUV = false;

	int m_nVertsSize;
};

