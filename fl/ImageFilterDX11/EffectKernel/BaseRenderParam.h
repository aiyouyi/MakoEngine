#pragma once
#include "HandDetectInterface.h"
#include "BodyDetectInterface.h"
#include "BaseDefine/Define.h"
#include "common.h"
#include "BaseDefine/Vectors.h"
//#include "Toolbox/DXUtils/DX11FBO.h"
//#include "Toolbox/DXUtils/DX11Texture.h"
//#include "Toolbox/DXUtils/DX11DoubleBuffer.h"
#include "Toolbox/Render/DoubleBufferRHI.h"
#include "Toolbox/Render/TextureRHI.h"
#include <vector>
#include "EffectDataDefine.h"

#define  MAX_SUPPORT_PEOPLE 5
#define  MAX_SUPPORT_HAND 6

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
	FaceOriImageRect pCameraRect;
	FaceExpression *pFaceExp;
};

struct BodyPosInfo
{
	float left;
	float top;
	float right;
	float bottom;
	float score;
};

struct HandInfo
{
	int handType;
	int gestureType;
	Vector4 handPoint[21];
	//handRect x:left y:top z:right w:bottom
	Vector4 handRect;
};

class BaseRenderParam
{
public:
	BaseRenderParam();
	~BaseRenderParam();
	void SetFaceNum(int nFaceCount);
	void SetSize(int width, int height);
	void SetFacePoint130(Vector2 *pPoint130,int nFaceIndex, int nWidth, int nHeight);
	void SetDoubleBuffer(std::shared_ptr<DoubleBufferRHI> pDoubleBuffer);
	void SetFacePosInfo(FacePosInfo *pFacePosInfo, int nFaceIndex);
	void SetBodyMaskTexture( std::shared_ptr<CC3DTextureRHI> pTex);
	void SetHairMaskTexture(std::shared_ptr<CC3DTextureRHI> pTex);
	//void SetSrcTex(ID3D11ShaderResourceView * tex);
	void SetSrcTex(std::shared_ptr<CC3DTextureRHI> tex);
	void SetCardMaskID(std::map<AnchorType, long long>& cardId);
	std::map<AnchorType, long long>& GetCardMaskID();

	void SetSplitScreenNum(int SplitType);
	int GetSplitScreenNum(bool &bMirror);

	void SetExpressionCoffes(std::vector<float>& coffes);
	std::vector<float>& GetExpressionCoffes();

	void SetHandInfo(ccHandRes* handRes);
	HandInfo* GetGestureInfo(int nIndex);
	int GetHandCount();

	void SetBodyPoint(ccBodyRes * bodyRes);

	BodyPosInfo *GetBodyPosInfo(int nBodyIndex);
	Vector2 *GetBodyPoint(int nBodyIndex);
	Vector2 *GetFacePoint(int nFaceIndex, FacePointType eFaceType = FACE_POINT_130, bool bNormalize = false);
	std::shared_ptr<DoubleBufferRHI> GetDoubleBuffer();
	int GetWidth();
	int GetHeight();
	int GetFaceCount();
	int GetBodyCount();
	BYTE *GetBGR_SRC();
	FacePosInfo *GetFaceInfo(int nFaceIndex);
	std::shared_ptr<CC3DTextureRHI> GetBodyMaskTexture();
	std::shared_ptr<CC3DTextureRHI> GetHairMaskTexture();
	std::shared_ptr<CC3DTextureRHI> GetSrcTex();

	BYTE *m_pBGRA_Src = NULL;
	BYTE *m_pBGR_Src = NULL;
	long runtime = 0;
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
	//std::vector<HandInfo>m_HeartGesture;
	HandInfo m_HeartGesture[MAX_SUPPORT_HAND];

	Vector2 m_pBodyPoint16[MAX_SUPPORT_PEOPLE][16];
	BodyPosInfo m_BodyPosInfo[MAX_SUPPORT_PEOPLE];

	bool b_SplitMirror = false;
	int m_SplitScreenNum = 1;
	int m_nHandCount = 0;
	int m_nFaceCount = 0;
	int m_nBodyCount = 0;
	int m_nWidth;
	int m_nHeight;

	std::shared_ptr<DoubleBufferRHI> m_pDoubleBuffer;

	std::shared_ptr<CC3DTextureRHI> m_pBodyMask;
	std::shared_ptr<CC3DTextureRHI> m_pHairMask;

	std::shared_ptr<CC3DTextureRHI> m_SrcTex;

	std::vector<float> m_Coffes;
	
	std::map<AnchorType,long long> m_CardMask;
};

