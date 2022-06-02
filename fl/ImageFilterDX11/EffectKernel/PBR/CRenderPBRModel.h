#pragma once
#include "EffectKernel/CEffectPart.h"
#include "Toolbox/Drawable.hpp"
#include "Toolbox/DXUtils/DXUtils.h"
#include "Toolbox/EffectModel.hpp"
#include "CC3DEngine/Skeleton/CC3DSkeleton.h"
#include "CC3DEngine/Material/CC3DFurMaterial.h"
#include <vector>
#include<map>
#include <chrono>
class CC3DSceneManage;
class CC3DPbrRender;
class CC3DTexture;
class PostProcessManager;
class CC3DExpressDrive;

struct MaterialInfo
{
	std::string MaterialName;
	std::string MaterialType;
	Drawable* Animation;
};

class CRenderPBRModel 
{
public:
	CRenderPBRModel();
	~CRenderPBRModel();
	bool ReadConfig(XMLNode& childNode, HZIP hZip = 0, char* pFilePath = NULL);
	bool Prepare(std::string &resourePath);
	void PreRender(BaseRenderParam& RenderParam);
	void Render(BaseRenderParam& RenderParam);
	void PostRender(BaseRenderParam& RenderParam);

	void Release();
	CC3DSceneManage *m_3DScene;

	std::string m_SwitchPalate;

	std::vector<dynamicBoneParameter> m_DyBone_array;

	DX11DoubleBuffer *m_DoubleBuffer = NULL;

	long m_runTime = 0;
private:


	DX11Shader* pLDR2HDR = nullptr;
	DX11Shader* pHDR2LDR = nullptr;

	std::shared_ptr<CC3DVertexBuffer> mVertexBuffer;
	std::shared_ptr<CC3DIndexBuffer> mIndexBuffer;


	vector<MaterialInfo>m_MatrialVideo;;

	CC3DPbrRender *m_Render;
	bool m_EnableWrite;
	int m_nCamIndex = 0;
	int m_AnimationType = 1;
	float *m_pHdrData = 0;
	float m_MatScale = 100.f;
	float m_MatScaleHead = 1.f;
	float m_rotateX = 0.0f;
	int hdrWidth = 0;
	int hdrHeight = 0;

	
	std::chrono::time_point<std::chrono::system_clock> lastTimeStamp;

	float LightDir[4] = { 0.5f, -1.0f, 1.0f, 1.f };
	float LightColor[4] = { 1.f, 1.0f, 1.0f, 1.f };
	float ambientStrength = 1.f;
	float ambientFur = -1;
	float RoughnessRate = 1.f;
	float gamma = 1.5;
	int gammaRM = 0;
	int materialType = 0;
	int ReverseY = 0;
	float AnimateRate = 1.f;

	float m_RotateIBL = 0;
	int m_EnableShadow = 0;
	int m_EnableOutLine = 0;

	PostProcessManager *m_postManager = NULL;;
	CC3DExpressDrive *m_ExpressDrive = NULL;

	float m_bloomAlpha = 2.f;
	float m_bloomradius = 2.f;
	float m_bloomBeginAlpha = 0.3f;
	int m_Bloomlooptime = 0;

	std::string m_RenderMaterialParams;

	FurConstBuffer fur_constbuffer;
	std::shared_ptr<CC3DTextureRHI> noiseTex = nullptr;
	std::shared_ptr<CC3DTextureRHI> lengthTex = nullptr;
	int numFurLayers = 20;
	std::shared_ptr<CC3DTextureRHI> hairShiftTex = nullptr;
	std::shared_ptr<CC3DTextureRHI> hairOutLineMaskTex = nullptr;
	std::shared_ptr<CC3DTextureRHI> DefOutLineMaskTex = nullptr;
};

