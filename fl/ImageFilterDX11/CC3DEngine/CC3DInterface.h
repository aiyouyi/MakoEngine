#pragma once
#include "Toolbox/inc.h"
#include "Toolbox/event.h"
#include "DX11ImageFilterDef.h"
#include "BaseDefine/Define.h"
#include "BaseDefine/Vectors.h"
#include "Render/TextureRHI.h"
#include "CC3DEngine/Material/FurConfig.h"
#include "CC3DEngine/Skeleton/DynamicBoneInfo.h"
#include "CC3DEngine/CC3DConfigStream.h"
class BaseRenderParam;
struct BoundingBox;

namespace CC3DImageFilter
{
	struct CC3DInterfacePrivate;

	class  DX11IMAGEFILTER_EXPORTS_CLASS CC3DInterface
	{
	public:
		CC3DInterface();
		~CC3DInterface();
		void LoadModel(const std::string& szFile);
		void LoadZIP(const std::string& szFile);
		void SetHDRPath(const std::string& hdrPath);
		void SetResourcePath(const std::string& resPath);
		void SetScale(float Scale);
		void SetRotate(float x, float y);
		void SetTranslate(float x, float y);
		void Render(int width, int height, std::shared_ptr<CC3DTextureRHI> RenderTargetTex);
		void LoadAnimateJson(const std::string& fileName);
		void SetRotateIBL(float x, float y);
		void SetHDRScale(float Scale);
		void SetHDRContrast(float Contrast);
		void SetMainLightInfo(const Vector3& Dir, const Vector3& Color, float LightStrength, float AmbientStrength);
		void SetGamma(float gamma);
		void SetEnableDiffuseSRGB(bool enable);
		void SetFurData(const FurConfigData& data);
		void SetAOOffset(float offset);
		void SetEnableShadow(bool enable);
		void AddDynamicBone(const std::string& boneName);
		void UpdateDynamicParam(const CC3DImageFilter::dynamicBoneParameter& config);
		void DeleteDynamicBone(const std::string& db_name);
		void ResetDynamicBone();
		void SetTonemappingType(const PBRToneMappingType& type);
		void SetBloomInfo(const BloomInfo& Bloom);
	public:
		core::event<void(const std::map<std::string, int>&) > sigAddSkeletonLayer;
	public:
		const ModelConfig& GetModeConfig()const;
		const FurConfigData& GetFurData()const;
	private:
		void Play();
		void SetInnerScale(float x, float y, float z);
		void SetInnerTranslate(float x, float y, float z);
		void SetInnerRotate(float x, float y, float z);
		const BoundingBox& GetFirstModelBox() const;
		void OnAddSkeletonLayer(const std::map<std::string, int>& bone);

	private:
		std::shared_ptr< CC3DInterfacePrivate> InterfacePrivate;
	};
};


