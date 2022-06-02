#pragma once
#include "CCHandGestureInterface.h"
#include "BaseDefine/Define.h"
#include "BaseDefine/Vectors.h"
#include "Toolbox/DXUtils/DX11FBO.h"
#include "Toolbox/DXUtils/DX11Texture.h"
#include "Toolbox/DXUtils/DX11DoubleBuffer.h"
#include <vector>
enum FacePointType
{
	FACE_POINT_130,
	FACE_POINT_106,
};
#define  MAX_SUPPORT_PEOPLE 5
#define  MAX_SUPPORT_HAND 10

struct FaceOriImageRect
{
	float  x;
	float  y;
	float  width;
	float  height;
};

struct FaceExpression
{
	bool blinkLeft = false;		  //眨眼 -- left
	bool blinkRight = false;	  //眨眼 -- right
	bool openMouse = false;   //张嘴
	bool shakeHead = false;   //摇头
	bool nodHead = false;     //点头
	bool raiseEyeBrow = false;//挑眉
	bool pout = false;        //嘟嘴
};

struct FacePosInfo
{
	float yaw;
	float pitch;
	float roll;
	float x, y, z;
	float arrWMatrix[16];
	float faceW, faceH;

	FaceOriImageRect pFaceRect;
	FaceExpression *pFaceExp;
};

struct HandInfo
{
	int handType;
	int gestureType;
	Vector2 handPoint[21];
	//handRect x:left y:top z:right w:bottom
	Vector4 handRect;
};

class BaseRenderParam
{
public:
	BaseRenderParam();
	~BaseRenderParam();
	void SetFaceNum(int nFaceCount);
	void SetFacePoint130(Vector2 *pPoint130,int nFaceIndex, int nWidth, int nHeight);
	void SetDoubleBuffer(DX11DoubleBuffer *pDoubleBuffer);
	void SetFacePosInfo(FacePosInfo *pFacePosInfo, int nFaceIndex);
	void SetBodyMaskTexture(DX11Texture *pTex);
	void SetHairMaskTexture(DX11Texture * pTex);
	void SetSrcTex(ID3D11ShaderResourceView * tex);

	void SetHandInfo(ccHGHandRes * handRes);
	std::vector<HandInfo>& GetHeartGestureInfo();

	Vector2 *GetFacePoint(int nFaceIndex, FacePointType eFaceType = FACE_POINT_130, bool bNormalize = false);
	DX11DoubleBuffer *GetDoubleBuffer();
	int GetWidth();
	int GetHeight();
	int GetFaceCount();
	int GetHandCount();
	BYTE *GetBGR_SRC();
	FacePosInfo *GetFaceInfo(int nFaceIndex);
	DX11Texture *GetBodyMaskTexture();
	DX11Texture *GetHairMaskTexture();
	ID3D11ShaderResourceView *GetSrcTex();

	BYTE *m_pBGRA_Src = NULL;
	BYTE *m_pBGR_Src = NULL;
private:
	void FacePointIndexTransfer(Vector2 *pSrcPoint130, Vector2 *pDstPoint106);
	void NoramlizePoint(Vector2 * pPoint, Vector2 *pDstPoint, int nPoint);

	Vector2 m_pPoint130[MAX_SUPPORT_PEOPLE][130];
	Vector2 m_pPoint106[MAX_SUPPORT_PEOPLE][106];

	Vector2 m_pPoint130_Normal[MAX_SUPPORT_PEOPLE][130];
	Vector2 m_pPoint106_Normal[MAX_SUPPORT_PEOPLE][106];

	bool m_bHasNormlize[MAX_SUPPORT_PEOPLE];
	bool m_bHasTranslate106[MAX_SUPPORT_PEOPLE];

	FacePosInfo m_FacePosInfo[MAX_SUPPORT_PEOPLE];
	std::vector<HandInfo>m_HeartGesture;

	int m_nHandCount;
	int m_nFaceCount;
	int m_nWidth;
	int m_nHeight;

	DX11DoubleBuffer *m_pDoubleBuffer;

	DX11Texture *m_pBodyMask;
	DX11Texture *m_pHairMask;

	ID3D11ShaderResourceView *m_SrcTex;

};

