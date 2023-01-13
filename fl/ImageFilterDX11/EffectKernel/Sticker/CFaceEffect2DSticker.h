#pragma once
#include "EffectKernel/CEffectPart.h"
#include "Algorithm/Face2DMesh/MTFace2DInterFace.h"
#include "Toolbox/Drawable.hpp"
#include "Algorithm/Matrices.h"
#include "Algorithm/MathUtils.h"
#include "EffectKernel/DXBasicSample.h"
#include "EffectKernel/VideoInfo.h"

enum FaceBodyCondition
{
	None			=0,
	MouthOpen		=1,
	MouthClose		=2,
};

struct StickerVedioInfo
{
	StickerVedioInfo()
	{
		StartFrameTime = 0;
		EndFrameTime = 9900000;
	}
	std::string Path = "";
	int StartFrameTime;
	long StartTime;
	int EndFrameTime;
	long EndTime;
	FaceBodyCondition Condition = FaceBodyCondition::None;
	VideoInfo Info;
	long long ID;	
};

class CFaceEffect2DSticker : public CEffectPart, protected DXBaicSample
{
public:
	CFaceEffect2DSticker();
	virtual ~CFaceEffect2DSticker();
	virtual void Release();
	virtual void* Clone();
	virtual bool ReadConfig(XMLNode& childNode, HZIP hZip = 0,char *pFilePath=NULL);
	virtual bool ReadConfig(XMLNode& childNode, const std::string &path);

	virtual bool Prepare();
	virtual void Render(BaseRenderParam &RenderParam);
	void Resize(int nWidth, int nHeight);

	Vector2 m_SrcRect[4];
	//Drawable *m_drawable;
	float m_anim_offset;
	int m_loopmode;
	float fps = 30;
private:

	void ReadConfig(XMLNode& childNode, HZIP hZip = 0, char *pFilePath = NULL, const std::string &path = "");
	bool WriteConfig(std::string &tempPath, XMLNode &root, HZIP dst, HZIP src);
	void MergeVertex(float * pVertex, float * pUV, int nVertex);

	int GetVideoIndex(BaseRenderParam& RenderParam,long Rumtime);
	bool FaceConditionMatch(FaceBodyCondition Condition, FacePosInfo* FaceInfo);

	ID3D11Buffer *m_VerticeBuffer[MAX_SUPPORT_PEOPLE];
	ID3D11Buffer *m_IndexBuffer;
	ID3D11Buffer*       m_pConstantBuffer;

	int m_ModelWidth = 930;
	int m_ModelHeight = 1240;
	int m_Width = 0;
	int m_Height = 0;
	Vector2 m_Vertices[4];
	float *m_pMergeVertex;
	int m_nVerts;
	Vector2 m_SrcAbsRect[4];
	Vector2 m_DstPoint[3];
	Vector2 m_SrcPoint[3] = {Vector2(0.386523f*930, 0.429125f*1240), Vector2(0.599048f*930, 0.427873f*1240), Vector2(0.498499f*930, 0.544121f*1240)};
	
	std::vector<float> g_TextureCoordinate = { 0.0f,0.0f,1.0f,0.0f,0.0f,1.0f,1.0f,1.0f };

	bool m_KeepShape = false;
	bool m_RemoveRoate = false;
	Vector2 m_SrcPointRect[3] = { Vector2(241,504.6), Vector2(241, 877.8), Vector2(678.4,877.8) };
	
	Vector2 m_FarModelSrcPoint[3] = { Vector2(441.5,79.87), Vector2(441.5, 153.84), Vector2(511.67,153.84) };
	bool b_FarModel = false;
	std::vector< StickerVedioInfo> AllVedioItems;
	int CurrentVedioIndex = -1;
};

