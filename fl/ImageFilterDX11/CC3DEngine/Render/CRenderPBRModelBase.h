#pragma once
#include "Toolbox/inc.h"
#include "Toolbox/Drawable.hpp"
#include "Toolbox/xmlParser.h"
#include "CC3DEngine/Skeleton/CC3DSkeleton.h"
#include "Material/FurDataDefine.h"
#include "CC3DEngine/CC3DConfigStream.h"

struct MaterialInfo
{
	std::string MaterialName;
	std::string MaterialType;
	Drawable* Animation;
};

namespace CC3DImageFilter
{
	class CC3DSceneManage;
}

class PostProcessManager;
class PostProcessManagerGL;
class CC3DExpressDrive;

class CRenderPBRModelBase
{
public:
	CRenderPBRModelBase();
	virtual ~CRenderPBRModelBase();

	bool ReadConfig(XMLNode& childNode, HZIP hZip = 0, char* pFilePath = NULL);
	bool CheckIfHasDynamicBone() const;
	void InitModelName(const std::string& ModelName);
private:
	void ParseMaterial(XMLNode& childNode, HZIP hZip = 0, char* pFilePath = NULL);
public:
	std::shared_ptr<CC3DImageFilter::CC3DSceneManage> m_3DScene;
	std::string m_SwitchPalate;

	virtual void CreatePostProcessManager() = 0;

public:
	
	int CamIndex = 0;
	std::string ModelName;
	std::shared_ptr<CC3DImageFilter::EffectConfig> EffectConfig ;

	std::string RenderMaterialParams;
	

	std::unordered_map<std::string, Vector4> HSVColors;

protected:
	float m_AnimateBlendTime = 0.05f;
	std::shared_ptr<float> m_pHdrData;
	
	int hdrWidth = 0;
	int hdrHeight = 0;

	std::vector<MaterialInfo> m_MatrialVideo;

	PostProcessManager* m_postManager = nullptr;
	PostProcessManagerGL* m_postManagerGL = nullptr;
	CC3DExpressDrive* m_ExpressDrive = nullptr;
};