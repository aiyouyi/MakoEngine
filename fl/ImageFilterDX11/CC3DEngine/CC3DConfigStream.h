#ifndef _CC_CONFIG_STREAM_H_
#define _CC_CONFIG_STREAM_H_

#include "DX11ImageFilterDef.h"
#include "RenderCommon/PBRRenderDef.h"
#include "CC3DEngine/Skeleton/DynamicBoneInfo.h"

struct XMLNode;

namespace CC3DImageFilter 
{
	struct PBRNoramlInfo;
	struct FurConfigData;

	struct PBRToneMappingType
	{
		int ToneMappingType = 1;
		float Contrast = 2.0;
		float Saturation = 1.5;
	};

	struct BloomInfo
	{
		bool EnableBloom = false;
		float BloomAlpha = 2.f;
		float Bloomradius = 2.f;
		float BloomBeginAlpha = 0.3f;
		int Bloomlooptime = 0;
		float BloomStrength = 1.0f;

		std::map<std::string, std::string> EmissiveMask;
	};

	//<anidrawable name="YanJing" offset="0" items="256/%06d.png" iteminfo="0,149,66,1"></anidrawable>  
	struct Anidrawable
	{
		std::string name;
		std::string offset;
		std::string items;
		std::string iteminfo;
		std::string loopMode;
		std::string genMipmaps;
	};

	struct ModelConfig
	{
		bool UseNewPBR = true;
		std::string GLBFileName;
		std::string HDRFileName;
		Vector3 LightDir = Vector3(0, 0, 1);
		Vector3 LightColor = Vector3(1, 1, 1);
		float LightStrength = 2.8f;
		float AmbientStrength = 0.74;
		float hdrRotateX = 0.0;
		float hdrRotateY = 0.0;
		float emissRadius = 1.0;
		float emissStrength = 0.0f;
		float AOOffset = 0.0f;
		float RoughnessRate = 1.0f;
		float AnimateRate = 1.0f;

		float Gamma = 1.5;
		SkinInfo skin_info;
		bool bEnableSkinRender = true;
		bool bUseBlurNormal = false;
		bool bEnableShadow = false;
		bool bEnableFxaa = true;
		bool bEnableHDR = false;
		bool bEnableSRGB = true;
		bool bEnableDepthWrite = true;
		bool bEnableOutLine = false;
		bool bEnableGammaRM = false;

		float shadowHue = 0.0f;
		float shadowSaturation = 0.0f;
		float shadowStrength = 1.0f;
		bool bEnablePostProcessColorAdjust;
		Vector4 postprocessColor;

		bool bEnableHairColor = false;
		Vector3 HairColorHSV = { 0.0, 0.0, 0.0 };
		std::unordered_map<std::string, PBRNoramlInfo> MapNormalInfo;
		std::unordered_map<std::string, Vector4> MapOutlineColor;
		float OutlineWidth = 10.0f;

		float SecondaryShift = 0.2;
		float PrimaryShift = 0.2;
		float SpecularPower = 50.0;
		float ShiftU = 1.0;
		float SpecularScale = 0.6;
		float SpecularWidth = 0.5;
		float SpecualrStrength = 1.0;
		Vector4 KajiyaSpecularColor = Vector4(1.0, 1.0, 1.0, 1.0);

		int AnimationType = 1;
		BloomInfo Bloom;
		float EmissStength = 1.0f;
		bool EnableEmiss = false;
		bool EnableExpress = false;
		float matScale = 1.0f;
		float matScaleHead = 1.0f;
		float HDRScale = 1.0f;
		float HDRContrast = 0.f;

		std::vector<CC3DImageFilter::dynamicBoneParameter> DyBone_array;

		PBRToneMappingType ToneMapping;

		Anidrawable Drawable;
	};

	class DX11IMAGEFILTER_EXPORTS_CLASS CC3DConfigStream
	{
	public:
		CC3DConfigStream() {}
		~CC3DConfigStream() {}
	public:
		//保存XML配置
		void SaveModelXMLConfig(const std::string& szFile, const ModelConfig& Info,const FurConfigData& FurData);
		void SaveModelXML(XMLNode& Root, const ModelConfig& Info, const FurConfigData& FurData);
	public:

		std::unordered_map<std::string, Vector3> mColorChangeMap;

	};
}
#endif