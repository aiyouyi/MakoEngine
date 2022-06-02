#pragma once
#include "EffectKernel/CEffectPart.h"
#include <vector>
#include "BaseDefine/Vectors.h"
#include "Algorithm/InterFacePointDeform.h"
#include "EffectKernel/FaceDeformation.h"
#include "EffectKernel/EllipseDeformation.h"

class MAGICSTUDIO_EXPORTS_CLASS CFaceSPMLS :public CEffectPart
{
public:
	CFaceSPMLS();
	virtual ~CFaceSPMLS();
	virtual void* Clone();
	virtual bool ReadConfig(XMLNode& childNode, HZIP hZip = 0, char *pFilePath = NULL);
	virtual bool ReadConfig(XMLNode& childNode, const std::string &path);
	virtual bool Prepare();
	virtual void Render(BaseRenderParam &RenderParam);
	virtual void Release();
	
	virtual void UpdateOffset();
	virtual float* GetOffsetTexture();
	virtual void UpdateWithOffestTexture(float *pOffestTexture);
	virtual void SetOperate(CFaceSPMLS *part);
private:
	
	void ReadConfig(XMLNode& childNode, HZIP hZip = 0, char *pFilePath = NULL, const std::string &path = "");
	bool WriteConfig(std::string &tempPath, XMLNode &root, HZIP dst, HZIP src);
	void Resize(int nWidth, int nHeight);
	void GenaratePoint(std::vector<Vector2> &point, std::vector<unsigned short> &vIndex, std::vector<Vector4> &Offest);
	void FilterToBigFBO();
	void RunFace106To118(Vector2 *pFacePoint, Vector2 *pFacePoint118);

	float CalArea(Vector2 point1, Vector2 point2, Vector2 point3);

	void GetFaceScaleEachPoint(Vector2 * pPoint118, Vector2 * pStandard442Src, unsigned short *pFaceMesh, Vector2 * pScaleValue442);
	//�����ȡ����ƫ��
	void GetOffsetTransform(Vector2 *pPoint118, int nImgWidth, int nImgHeight, float *pOutMat);

	float GetSideFaceParam(float * pPoint118, float SideParam);

	void MergeVertex(float * pVertex, float * FaceScale,  float *pMaskUV, int nVertex);

public:
	FaceDeformation *m_FaceShape = NULL;
	EllipseDeformation *m_EllipseShape = NULL;
	std::string m_Style = "Mesh";
	bool m_hasCopy = false;
private:
	
	BYTE *m_bOffestTexture;
	int m_SmallTextureWidth;
	int m_SmallTextureHeight;
	DX11FBO *m_pBigOffestFBO;
	DX11Texture *m_pBigoffestTexture;
	float *m_pOffestTexture;
	int m_nBigSize;
	int m_nWidth;
	int m_nHeight;
	int m_VTPointCount;
	int m_VTTriangleSize = 0;

	DX11Shader *m_pShaderFilterBig;
	ID3D11Buffer *m_IndexBuffer;
	ID3D11Buffer *m_VerticeBuffer[MAX_SUPPORT_PEOPLE];
	ID3D11Buffer *m_VerticeBuffer2;
	ID3D11Buffer *m_IndexBuffer2;
	ID3D11Buffer* m_pConstantBuffer;


	InterFacePointDeform *m_InterFM;

	float *m_pMergeVertex;
	int m_nVertsSize;

	bool m_needUpdate = true;
};

