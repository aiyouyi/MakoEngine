#pragma once
#include "EffectKernel/CEffectPart.h"
#include "Toolbox/Drawable.hpp"

#define MAX_EFFECT_TYPE_NUM 3

enum EffectType
{
	FIRE = 0,
	STATIC_FIREWORK,
	CRACK_FIREWORK,
};

//struct EffectInfo
//{
//	EffectType m_EffectType;
//	float m_BoxRate[4];
//	long long m_anim_id;
//};

struct HandData
{
	Vector2 m_pVertices[4];
	Vector2 m_pUV[4];
	int m_nVerts;
	unsigned short *m_pTriangle;
	int m_nTriangle;

};
class CHandEffectHeart : public CEffectPart
{
public:
	CHandEffectHeart();
	~CHandEffectHeart();
	virtual void Release();
	virtual bool ReadConfig(XMLNode& childNode, HZIP hZip = 0, char *pFilePath = NULL);
	virtual bool ReadConfig(XMLNode& childNode, const std::string &path);
	virtual bool Prepare();
	virtual void Render(BaseRenderParam &RenderParam);

	
	
private:
	void ReadConfig(XMLNode& childNode, HZIP hZip = 0, char *pFilePath = NULL, const std::string &path = "");
	HandData *GenerateHandBox(Vector2 *pHandPoint, float* pBoxRate, int t);
	HandData *GenerateRect(float *pBoxRate);
	void MergeVertex(float *pVertex, float *pUV, int nVertex);
	
	float m_BoxRate[4];
	//std::vector<EffectInfo>m_vEffectInfo;

	ID3D11Buffer *m_VerticeBuffer[MAX_SUPPORT_HAND];
	ID3D11Buffer *m_IndexBuffer;
	
	long StartTime = -1;
	bool isCrack;
	bool isCrackFinished;

	int m_Width = 0;
	int m_Height = 0;
	HandData *m_pBox;
	float *m_pMergeVertex;
	std::vector<float> m_BoxUV = { 0.0f,0.0f,1.0f,0.0f,0.0f,1.0f,1.0f,1.0f };
	std::vector<unsigned short> m_BoxIndex= {0, 1, 2, 1, 2, 3};
};