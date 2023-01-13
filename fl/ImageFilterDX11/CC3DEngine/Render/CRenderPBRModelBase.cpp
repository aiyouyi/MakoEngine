#include "CRenderPBRModelBase.h"
#include "CC3DEngine/Common/CC3DEnvironmentConfig.h"
#include "CC3DEngine/Scene/CC3DSceneManage.h"
#include "CC3DEngine/FaceExpressDrive/CC3DExpressDrive.h"
#include "BaseDefine/json.hpp"
#include "BaseDefine/commonFunc.h"

CRenderPBRModelBase::CRenderPBRModelBase()
{
	m_3DScene = nullptr;
	m_pHdrData = nullptr;
	std::lock_guard<std::mutex> Lock(CC3DEnvironmentConfig::getInstance()->MapEffectLock);
	EffectConfig = CC3DEnvironmentConfig::getInstance()->MapEffectConfig["Empty"];
}

CRenderPBRModelBase::~CRenderPBRModelBase()
{


}

bool CRenderPBRModelBase::ReadConfig(XMLNode& childNode, HZIP hZip /*= 0*/, char* pFilePath /*= NULL*/)
{
	if (!childNode.isEmpty())
	{

		//model
		XMLNode nodeGltfModel = childNode.getChildNode("gltf", 0);
		if (!nodeGltfModel.isEmpty())
		{
			const char* szModelName = nodeGltfModel.getAttribute("model");
			if (szModelName)
			{
				InitModelName(szModelName);
			}

			auto& Config = EffectConfig->ModelConfig;

			const char* szEnableWrite = nodeGltfModel.getAttribute("enablewrite");
			if (szEnableWrite != nullptr && !strcmp(szEnableWrite, "false"))
			{
				Config.bEnableDepthWrite = false;
			}

			nodeGltfModel.getAttributeIntValue("camIndex", CamIndex);

			nodeGltfModel.getAttributeFloatValue("matScale", Config.matScale);
			nodeGltfModel.getAttributeFloatValue("matScaleHead", Config.matScaleHead);
			nodeGltfModel.getAttributeFloatValue("rotateX", Config.hdrRotateX);

			int UseNewPBR = 0;
			nodeGltfModel.getAttributeIntValue("NewPBR", UseNewPBR);
			Config.UseNewPBR = UseNewPBR == 1;

			const char* szHdrName = nodeGltfModel.getAttribute("hdr");
			if (szHdrName != nullptr)
			{
				Config.HDRFileName = szHdrName;

				m_pHdrData = CreateHDRFromZIP(hZip, szHdrName, hdrWidth, hdrHeight);
				
			}


			const char* szHairShift = nodeGltfModel.getAttribute("hair_shift");
			if (szHairShift)
			{
				EffectConfig->HairShiftTex = GetDynamicRHI()->CreateAsynTextureZIP(hZip, szHairShift);
			}

			const char* szHairOutLineMask = nodeGltfModel.getAttribute("OutlineMask");
			if (szHairOutLineMask)
			{
				EffectConfig->HairOutLineMaskTex = GetDynamicRHI()->CreateAsynTextureZIP(hZip, szHairOutLineMask);
			}

			
			if (szModelName != nullptr)
			{
				Config.GLBFileName = szModelName;
				int index;
				ZIPENTRY ze;

				if (FindZipItem(hZip, szModelName, true, &index, &ze) == ZR_OK)
				{
					char* pDataBuffer = new char[ze.unc_size];
					ZRESULT res = UnzipItem(hZip, index, pDataBuffer, ze.unc_size);
					if (res == ZR_OK)
					{
						if (m_3DScene == nullptr)
						{
							m_3DScene = std::make_shared<CC3DImageFilter::CC3DSceneManage>();
							
						}
						CC3DEnvironmentConfig::getInstance()->SceneMgr = m_3DScene;
						m_3DScene->LoadMainModel((char*)szModelName, (const unsigned char*)pDataBuffer, ze.unc_size);
					}
					SAFE_DELETE_ARRAY(pDataBuffer);
				}
			}

			const char* szAnimationBin = nodeGltfModel.getAttribute("AnimationBin");
			if (szAnimationBin != nullptr)
			{
				int index;
				ZIPENTRY ze;

				if (FindZipItem(hZip, szAnimationBin, true, &index, &ze) == ZR_OK)
				{
					char* pDataBuffer = new char[ze.unc_size];
					ZRESULT res = UnzipItem(hZip, index, pDataBuffer, ze.unc_size);
					if (res == ZR_OK && m_3DScene)
					{
						m_3DScene->ReadSkeletonAnimateData(pDataBuffer);
					}
					SAFE_DELETE_ARRAY(pDataBuffer);
				}
			}


			Config.AnimationType = 1;
			nodeGltfModel.getAttributeIntValue("AnimationType", Config.AnimationType);

			nodeGltfModel.getAttributeVectorValue("LightDir", Config.LightDir[0], Config.LightDir[1], Config.LightDir[2], Config.LightStrength);
			nodeGltfModel.getAttributeVectorValue("LightColor", Config.LightColor);
			nodeGltfModel.getAttributeFloatValue("ambientStrength", Config.AmbientStrength);
			//
			nodeGltfModel.getAttributeFloatValue("AoOffset", Config.AOOffset);

			nodeGltfModel.getAttributeFloatValue("RoughnessRate", Config.RoughnessRate);
			nodeGltfModel.getAttributeFloatValue("AnimateRate", Config.AnimateRate);
			nodeGltfModel.getAttributeFloatValue("AnimateBlendTime", m_AnimateBlendTime);

			const char*szInfo = nodeGltfModel.getAttribute("SwitchPalate");
			if (szInfo != nullptr)
			{
				m_SwitchPalate = szInfo;
			}

			nodeGltfModel.getAttributeFloatValue("rotateIBLX", Config.hdrRotateX);
			nodeGltfModel.getAttributeFloatValue("RotateIBLY", Config.hdrRotateY);
			nodeGltfModel.getAttributeFloatValue("HdrScale", Config.HDRScale);
			nodeGltfModel.getAttributeFloatValue("HDRContrast", Config.HDRContrast);

			int EnableShadow = 0;
			nodeGltfModel.getAttributeIntValue("EnableShadow", EnableShadow);
			Config.bEnableShadow = EnableShadow == 1;

			nodeGltfModel.getAttributeFloatValue("gamma", Config.Gamma);

			int EnableGammaRM = 0;
			nodeGltfModel.getAttributeIntValue("gammaRM", EnableGammaRM);
			Config.bEnableGammaRM = EnableGammaRM == 1;

			int ExpressionEnable = 0;
			nodeGltfModel.getAttributeIntValue("EnableExpress", ExpressionEnable);
			Config.EnableExpress = ExpressionEnable == 1;

			if (Config.EnableExpress)
			{
				m_ExpressDrive = new CC3DExpressDrive();
			}
			
		}

		XMLNode ChangeColor = childNode.getChildNode("ChangeColor", 0);
		if (!ChangeColor.isEmpty())
		{
			int MatieralCount = 0;
			const char* szInfo = ChangeColor.getAttribute("NameCount");
			if (szInfo != NULL)
			{
				sscanf(szInfo, "%d", &MatieralCount);
			}

			for (int index = 1; index <= MatieralCount; ++index)
			{
				char szMateiralName[128] = { 0 };
				sprintf_s(szMateiralName, "Name%d", index);
				std::string MaterialName;
				szInfo = ChangeColor.getAttribute(szMateiralName);
				if (szInfo != NULL)
				{
					MaterialName = szInfo;
					char szHSVName[128] = { 0 };
					sprintf_s(szHSVName, "HSV%d", index);

					Vector4 Color;
					szInfo = ChangeColor.getAttribute(szHSVName);
					if (szInfo != NULL)
					{
						sscanf(szInfo, "%f,%f,%f,%f", &Color.x, &Color.y, &Color.z, &Color.w);
					}

					HSVColors.insert({ MaterialName,Color });
				}
			}
		}

		XMLNode nodePost = childNode.getChildNode("Post", 0);
		if (!nodePost.isEmpty())
		{
			auto& Config = EffectConfig->ModelConfig;

			Config.Bloom.EnableBloom = true;
			
			const char* szInfo = nodePost.getAttribute("bloom");
			if (szInfo != NULL)
			{
				sscanf(szInfo, "%f,%f,%d", &Config.Bloom.BloomAlpha, &Config.Bloom.Bloomradius, &Config.Bloom.Bloomlooptime);
			}

			nodePost.getAttributeFloatValue("BloomStrength", Config.Bloom.BloomStrength);
			nodePost.getAttributeFloatValue("bloomBegin", Config.Bloom.BloomBeginAlpha);

			EffectConfig->EmissiveMaskTex.clear();
			int i = -1;
			XMLNode EmissiveMaskNode = nodePost.getChildNode("EmissiveMask", ++i);
			while (!EmissiveMaskNode.isEmpty())
			{
				const char* szName = EmissiveMaskNode.getAttribute("name");
				const char* szTex = EmissiveMaskNode.getAttribute("tex");

				if (szName && szTex)
				{
					EffectConfig->ModelConfig.Bloom.EmissiveMask.insert({ szName,szTex });
					auto MaterialTex = GetDynamicRHI()->CreateAsynTextureZIP(hZip, szTex);

					EffectConfig->EmissiveMaskTex.insert({ szName,MaterialTex });
				}

				EmissiveMaskNode = nodePost.getChildNode("EmissiveMask", ++i);
			}

		}
		else
		{
			EffectConfig->ModelConfig.Bloom.EnableBloom = false;
		}

		std::vector< CC3DImageFilter::dynamicBoneParameter > DyBone_array;
		int i = -1;
		XMLNode nodeDyBone = childNode.getChildNode("DyBone", ++i);
		while (!nodeDyBone.isEmpty())
		{
			CC3DImageFilter::dynamicBoneParameter Param;
			const char* szName = nodeDyBone.getAttribute("name");
			if (szName != NULL)
			{
				Param.bone_name = szName;
			}
			nodeDyBone.getAttributeFloatValue("Damping", Param._fDamping);
			nodeDyBone.getAttributeFloatValue("Elasticity", Param._fElasticity);
			nodeDyBone.getAttributeFloatValue("Stiffness", Param._fStiffness);

			nodeDyBone.getAttributeFloatValue("Inert", Param._fInert);
			nodeDyBone.getAttributeFloatValue("Radius", Param._fRadius);

			nodeDyBone.getAttributeFloatValue("EndLength", Param._fEndLength);

			DyBone_array.push_back(Param);
			nodeDyBone = childNode.getChildNode("DyBone", ++i);
		}
		EffectConfig->ModelConfig.DyBone_array = DyBone_array;
		if (m_3DScene != NULL && m_3DScene->m_Skeleton.size() > 0 && m_3DScene->m_Skeleton[0]->m_RootNode.size() > 0 && !DyBone_array.empty())
		{
			m_3DScene->m_Skeleton[0]->AddDynamicBone(DyBone_array);
		}

		i = -1;
		XMLNode nodeAnime = childNode.getChildNode("skeletalAnimation", ++i);
		while (!nodeAnime.isEmpty())
		{
			const char* szAnimateName = nodeAnime.getAttribute("name");
			if (szAnimateName != NULL)
			{
				int index;
				ZIPENTRY ze;

				if (FindZipItem(hZip, szAnimateName, true, &index, &ze) == ZR_OK)
				{
					char* pDataBuffer = new char[ze.unc_size];
					ZRESULT res = UnzipItem(hZip, index, pDataBuffer, ze.unc_size);
					if (res == ZR_OK && m_3DScene)
					{
						m_3DScene->AddSkeletonAnnimateData(pDataBuffer, szAnimateName);
					}
					SAFE_DELETE_ARRAY(pDataBuffer);
				}
			}

			const char* szBlendName = nodeAnime.getAttribute("blendAnim");
			if (szBlendName != nullptr)
			{
				int index;
				ZIPENTRY ze;

				if (FindZipItem(hZip, szBlendName, true, &index, &ze) == ZR_OK)
				{
					char* pDataBuffer = new char[ze.unc_size];
					ZRESULT res = UnzipItem(hZip, index, pDataBuffer, ze.unc_size);
					if (res == ZR_OK && m_3DScene)
					{
						const char* szBoneName = nodeAnime.getAttribute("boneName");
						std::string boneName;
						if (szBoneName != nullptr)
						{
							boneName = szBoneName;
						}
						const char* szInfo = nodeAnime.getAttribute("blendWeight");
						float blendWeight = 1.0f;
						if (szInfo != nullptr)
						{
							sscanf(szInfo, "%f", &blendWeight);
						}
						m_3DScene->AddSkeletonAnnimateDataForBlend(pDataBuffer, szAnimateName, boneName, blendWeight);
					}
					SAFE_DELETE_ARRAY(pDataBuffer);
				}
			}

			nodeAnime = childNode.getChildNode("skeletalAnimation", ++i);
		}

		//µ•’≈Ã˘Õº
		i = -1;
		XMLNode nodeDrawable = childNode.getChildNode("drawable", ++i);
		char szFullFile[256];
		while (!nodeDrawable.isEmpty())
		{
			MaterialInfo mv;
			mv.MaterialType = "Effect";

			const char* szDrawableName = nodeDrawable.getAttribute("name");
			mv.MaterialName = szDrawableName;

			const char* szImagePath = nodeDrawable.getAttribute("image");

			sprintf(szFullFile, "%s/%s", pFilePath, szImagePath);

			const char* szGenMipmap = nodeDrawable.getAttribute("genMipmaps");
			bool bGenMipmap = false;
			if (szGenMipmap != NULL && (strcmp(szGenMipmap, "yes") || strcmp(szGenMipmap, "YES")))
			{
				bGenMipmap = true;
			}

			std::shared_ptr<MaterialTexRHI> TexRHI = GetDynamicRHI()->CreateAsynTextureZIP(hZip, szImagePath, bGenMipmap);
			mv.Animation = new BitmapDrawable(TexRHI);

			m_MatrialVideo.push_back(mv);
			nodeDrawable = childNode.getChildNode("drawable", ++i);
		}
		//–Ú¡–÷°
		i = -1;
		nodeDrawable = childNode.getChildNode("anidrawable", ++i);
		while (!nodeDrawable.isEmpty())
		{
			MaterialInfo mv;
			mv.MaterialType = "Effect";
			if (!nodeDrawable.isEmpty()) {
				const char* szDrawableName = nodeDrawable.getAttribute("name");
				EffectConfig->ModelConfig.Drawable.name = szDrawableName;
				const char* szDrawabletype = nodeDrawable.getAttribute("type");
				mv.MaterialName = szDrawableName;
				AnimationDrawable* drawable = new AnimationDrawable();
				if (szDrawabletype != NULL)
				{
					mv.MaterialType = szDrawabletype;
				}

				const char* szOffset = nodeDrawable.getAttribute("offset");
				long offset = atol(szOffset);
				drawable->setOffset(offset);
				EffectConfig->ModelConfig.Drawable.offset = szOffset ? szOffset : "0";

				const char* szLoopMode = nodeDrawable.getAttribute("loopMode");
				if (szLoopMode != NULL && strcmp(szLoopMode, "oneShot") == 0)
					drawable->setLoopMode(ELM_ONESHOT);
				else
					drawable->setLoopMode(ELM_REPEAT);
				EffectConfig->ModelConfig.Drawable.loopMode = szLoopMode ? szLoopMode : "";

				const char* szGenMipmap = nodeDrawable.getAttribute("genMipmaps");
				bool bGenMipmap = false;
				if (szGenMipmap != NULL && (strcmp(szGenMipmap, "yes") || strcmp(szGenMipmap, "YES")))
					bGenMipmap = true;
				EffectConfig->ModelConfig.Drawable.genMipmaps = szGenMipmap ? szGenMipmap : "";

				const char* szItems = nodeDrawable.getAttribute("items");
				const char* szItemInfo = nodeDrawable.getAttribute("iteminfo");
				if (szItems != NULL && szItemInfo != NULL)
				{
					EffectConfig->ModelConfig.Drawable.items = szItems;
					EffectConfig->ModelConfig.Drawable.iteminfo = szItemInfo;
					int iStart = 0;
					int nCount = 0;
					int nDuring = 0;
					int nStep = 1;
					char szImagePath[128];
					sscanf(szItemInfo, "%d,%d,%d,%d", &iStart, &nCount, &nDuring, &nStep);
					if (nCount > 0 && nDuring > 0)
					{
						char szFullFile[256];
						for (; iStart <= nCount; iStart += nStep)
						{
							sprintf(szImagePath, szItems, iStart);

							sprintf(szFullFile, "%s/%s", pFilePath, szImagePath);

							std::shared_ptr<MaterialTexRHI> TexRHI = GetDynamicRHI()->CreateAsynTextureZIP(hZip, szImagePath, bGenMipmap);

							long during = nDuring;

							drawable->appandTex(during, TexRHI);
						}
					}
					mv.Animation = drawable;
				}

				m_MatrialVideo.push_back(mv);
				nodeDrawable = childNode.getChildNode("anidrawable", ++i);
			}
		}

		XMLNode TonemappingNode = childNode.getChildNode("Tonemapping", 0);
		if (!TonemappingNode.isEmpty())
		{
			TonemappingNode.getAttributeIntValue("Type", EffectConfig->ModelConfig.ToneMapping.ToneMappingType);
			TonemappingNode.getAttributeFloatValue("Contrast", EffectConfig->ModelConfig.ToneMapping.Contrast);
			TonemappingNode.getAttributeFloatValue("Saturation", EffectConfig->ModelConfig.ToneMapping.Saturation);
		}

		ParseMaterial(childNode, hZip, pFilePath);

		return true;
	}
	return false;
}

bool CRenderPBRModelBase::CheckIfHasDynamicBone() const
{
	if (!m_3DScene || m_3DScene->m_Skeleton.empty())
	{
		return false;
	}
	return m_3DScene->m_Skeleton[0]->HasDynamicBone();
}

void CRenderPBRModelBase::InitModelName(const std::string& Name)
{
	ModelName = Name;
	std::lock_guard<std::mutex> Lock(CC3DEnvironmentConfig::getInstance()->MapEffectLock);
	if (CC3DEnvironmentConfig::getInstance()->MapEffectConfig[ModelName])
	{
		EffectConfig = CC3DEnvironmentConfig::getInstance()->MapEffectConfig[ModelName];
	}
	else
	{
		EffectConfig = std::make_shared<CC3DImageFilter::EffectConfig>();
		CC3DEnvironmentConfig::getInstance()->MapEffectConfig[ModelName] = EffectConfig;
	}

}

void CRenderPBRModelBase::ParseMaterial(XMLNode& childNode, HZIP hZip /*= 0*/, char* pFilePath /*= NULL*/)
{
	
	XMLNode MaterialNode = childNode.getChildNode("MaterialJson", 0);
	if (!MaterialNode.isEmpty())
	{
		int index;
		ZIPENTRY ze;
		const char* szInfo = MaterialNode.getAttribute("Name");
		if (ZR_OK == FindZipItem(hZip, szInfo, true, &index, &ze))
		{
			char* jsonBuffer = new char[ze.unc_size];
			ZRESULT res = UnzipItem(hZip, index, jsonBuffer, ze.unc_size);
			if (res != ZR_OK)
			{
				delete[]jsonBuffer;
				CloseZip(hZip);
			}
			nlohmann::json dyRoot= nlohmann::json::parse(jsonBuffer, jsonBuffer + ze.unc_size);
			
			delete[] jsonBuffer;
			if (!dyRoot.is_null())
			{
				RenderMaterialParams = dyRoot.dump();
			}
		}
	}

	
	EffectConfig->FurData.ParseMaterial(childNode, hZip, pFilePath);
}
