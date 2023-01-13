#if defined(_WIN32)
#include "CC3DConfigStream.h"
#include "BaseDefine/commonFunc.h"
#include "CC3DEngine/Material/CC3DPbrMaterial.h"
#include "RenderCommon/PBRRenderDef.h"
#include "Toolbox/xmlParser.h"
#include "Toolbox/string_util.h"
#include "CC3DEngine/Material/FurConfig.h"

namespace CC3DImageFilter 
{

	void CC3DConfigStream::SaveModelXMLConfig(const std::string& szFile, const ModelConfig& Info, const FurConfigData& FurData)
	{
		XMLNode Doc;
		XMLNode Root = Doc.createXMLTopNode("models");
		Root.addAttribute("version", "1.0");
		SaveModelXML(Root, Info, FurData);

		Root.writeToFile(szFile.c_str());
	}

	void CC3DConfigStream::SaveModelXML(XMLNode& Root, const ModelConfig& Info, const FurConfigData& FurData)
	{
		XMLNode TypeEffectNode = Root.addChild("typeeffect");
		TypeEffectNode.addAttribute("type", "FacePBRModel");

		//<gltf model="EeDuo.glb" hdr="venice_sunset_1k.hdr" AnimationType="0" ambientStrength="1.58"  LightDir="0.0,0.0,1.0,1.7" RoughnessRate="1.0" AnimateRate="1.0" rotateIBL="0" rotateIBLY="0" EnableShadow="1" AoOffset="0.5"/>
		XMLNode GLTFNode = TypeEffectNode.addChild("gltf");
		GLTFNode.addAttribute("model", Info.GLBFileName.c_str());
		GLTFNode.addAttribute("hdr", Info.HDRFileName.c_str());
		GLTFNode.addAttribute("NewPBR", "1");
		GLTFNode.addAttribute("RoughnessRate", "1"); //set default value
		GLTFNode.addAttribute("AnimateRate", "1"); //set default value
		GLTFNode.addAttribute("AnimationType", core::format(Info.AnimationType).c_str());
		GLTFNode.addAttribute("LightDir", core::format_printf("%0.1f,%0.1f,%0.1f,%0.1f", Info.LightDir.x, Info.LightDir.y, Info.LightDir.z, Info.LightStrength).c_str());
		GLTFNode.addAttribute("gamma", core::format(Info.Gamma).c_str());
		GLTFNode.addAttribute("rotateIBLX", core::format(Info.hdrRotateX).c_str());
		GLTFNode.addAttribute("rotateIBLY", core::format(Info.hdrRotateY).c_str());
		GLTFNode.addAttribute("HdrScale", core::format(Info.HDRScale).c_str());
		GLTFNode.addAttribute("HdrContrast", core::format(Info.HDRContrast).c_str());
		GLTFNode.addAttribute("EnableShadow", Info.bEnableShadow ? "1" : "0");
		GLTFNode.addAttribute("AoOffset", core::format(Info.AOOffset).c_str());
		if (!Info.bEnableDepthWrite)
		{
			GLTFNode.addAttribute("enablewrite", "false");
		}
		GLTFNode.addAttribute("EnableExpress", core::format(Info.EnableExpress).c_str());
		if (Info.matScale > 1.1)
		{
			GLTFNode.addAttribute("matScale", core::format(Info.matScale).c_str());
			GLTFNode.addAttribute("matScaleHead", core::format(Info.matScale).c_str());
		}


		//<Tonemapping type="1" contrast=2.0 saturation="1.5"/>
		XMLNode TonemappingNode = TypeEffectNode.addChild("Tonemapping");
		TonemappingNode.addAttribute("Type", core::format(Info.ToneMapping.ToneMappingType).c_str());
		TonemappingNode.addAttribute("Contrast", core::format(Info.ToneMapping.Contrast).c_str());
		TonemappingNode.addAttribute("Saturation", core::format(Info.ToneMapping.Saturation).c_str());

		//<Post bloom="1.5,1.0,200"></Post>
		if (Info.Bloom.EnableBloom)
		{
			XMLNode BloomNode = TypeEffectNode.addChild("Post");
			BloomNode.addAttribute("bloom", core::format_printf("%0.1f,%0.1f,%d", Info.Bloom.BloomAlpha, Info.Bloom.Bloomradius, Info.Bloom.Bloomlooptime).c_str());
			BloomNode.addAttribute("BloomStrength", core::format(Info.Bloom.BloomStrength).c_str());
			//BloomNode.addAttribute("EmissiveMask", Info.Bloom.EmissiveMask.c_str());

			for (auto it = Info.Bloom.EmissiveMask.begin(); it != Info.Bloom.EmissiveMask.end(); ++it)
			{
				XMLNode EmissiveMaskNode = BloomNode.addChild("EmissiveMask");
				EmissiveMaskNode.addAttribute("name", it->first.c_str());
				EmissiveMaskNode.addAttribute("tex", it->second.c_str());
			}
		}

		//<FurMaterial name="ErDuo_fur"  NoiseTex="noise.jpg"  FurLength="0.5" FurAmbientStrength="3" FurLevel="40" UVScale="30" Gravity="0,-1,0" FurLightExposure="0.4" Fresnel="2.0" FurMask="0.5" Tming="0.5" ></FurMaterial>
		if (!FurData.FurMaterialName.empty())
		{
			XMLNode FurMaterialNode = TypeEffectNode.addChild("FurMaterial");
			FurMaterialNode.addAttribute("name", FurData.FurMaterialName.c_str());
			FurMaterialNode.addAttribute("NoiseTex", FurData.NoiseTex.c_str());
			FurMaterialNode.addAttribute("FurLength", core::format(FurData.FurLength).c_str());
			FurMaterialNode.addAttribute("LengthTex", FurData.LengthTexName.c_str());
			FurMaterialNode.addAttribute("FurAmbientStrength", core::format(FurData.FurAmbientStrength).c_str());
			FurMaterialNode.addAttribute("FurLevel", core::format(FurData.FurLevel).c_str());
			FurMaterialNode.addAttribute("UVScale", core::format(FurData.UVScale).c_str());
			FurMaterialNode.addAttribute("Gravity", core::format_printf("%0.1f,%0.1f,%0.1f", FurData.vGravity.x, FurData.vGravity.y, FurData.vGravity.z).c_str());
			FurMaterialNode.addAttribute("FurLightExposure", core::format(FurData.FurLightExposure).c_str());
			FurMaterialNode.addAttribute("Fresnel", core::format(FurData.Fresnel).c_str());
			FurMaterialNode.addAttribute("FurMask", core::format(FurData.FurMask).c_str());
			FurMaterialNode.addAttribute("Tming", core::format(FurData.Tming).c_str());
			FurMaterialNode.addAttribute("LightFilter", core::format(FurData.LightFilter).c_str());
		}

		if (!Info.Drawable.name.empty())
		{
			XMLNode AnidrawableNode = TypeEffectNode.addChild("anidrawable");
			AnidrawableNode.addAttribute("name", Info.Drawable.name.c_str());
			AnidrawableNode.addAttribute("offset", Info.Drawable.offset.c_str());
			if (!Info.Drawable.items.empty())
			{
				AnidrawableNode.addAttribute("items", Info.Drawable.items.c_str());
			}
			if (!Info.Drawable.iteminfo.empty())
			{
				AnidrawableNode.addAttribute("iteminfo", Info.Drawable.iteminfo.c_str());
			}
			if (!Info.Drawable.loopMode.empty())
			{
				AnidrawableNode.addAttribute("loopMode", Info.Drawable.loopMode.c_str());
			}
			if (!Info.Drawable.genMipmaps.empty())
			{
				AnidrawableNode.addAttribute("genMipmaps", Info.Drawable.genMipmaps.c_str());
			}
		}

		//<DyBone name="bianzi_1"  Damping="0.2" Elasticity="0.06" Stiffness="0.5" Inert="0.5"></DyBone>  
		for (const auto& DyBone : Info.DyBone_array)
		{
			XMLNode DyBoneNode = TypeEffectNode.addChild("DyBone");
			DyBoneNode.addAttribute("name", DyBone.bone_name.c_str());
			DyBoneNode.addAttribute("Damping", core::format(DyBone._fDamping).c_str());
			DyBoneNode.addAttribute("Elasticity", core::format(DyBone._fElasticity).c_str());
			DyBoneNode.addAttribute("Stiffness", core::format(DyBone._fStiffness).c_str());
			DyBoneNode.addAttribute("Inert", core::format(DyBone._fInert).c_str());
		}
	}

}
#endif
