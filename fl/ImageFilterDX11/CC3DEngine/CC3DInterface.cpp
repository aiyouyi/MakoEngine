#include "CC3DInterface.h"
#include "Scene/CC3DSceneManage.h"
#include "Render/CC3DPbrRender.h"
#include "Render/CC3DPostProcessManager.h"
#include "FaceExpressDrive/CC3DFaceCustomization.h"
#include "Toolbox/Render/DynamicRHI.h"
#include "EffectKernel/BaseRenderParam.h"
#include "Toolbox/DXUtils/DX11DoubleBufferWrapper.h"
#include "CC3DEngine/Material/CC3DMaterial.h"
#include "Common/CC3DEnvironmentConfig.h"
#include "Toolbox/vec2.h"
#include "CC3DEngine/Render/CRenderPBRModel.h"
#include "string_util.h"

const float KSizeScale = 300.f;

namespace CC3DImageFilter
{ 

	struct CC3DInterfacePrivate
	{
		std::shared_ptr<CC3DImageFilter::CC3DSceneManage> SceneMgr;
		std::shared_ptr<DoubleBufferRHI> RenderTargetDBuffer;
		CRenderPBRModel  RenderModel;
		std::string ResourcePath;
		core::vec2i Size;
		uint64_t StartTime = 0;
		float Scale = 1.0;
		core::vec2f Translate{0.0,0.0};
		core::vec2f Rotate{ 0.0,0.0 };
		bool HasPrepared = false;

	};

	CC3DInterface::CC3DInterface()
		:InterfacePrivate(std::make_shared<CC3DInterfacePrivate>())
	{
		InterfacePrivate->SceneMgr = std::make_shared<CC3DImageFilter::CC3DSceneManage>();
		InterfacePrivate->SceneMgr->sigAddSkeletonLayer.bind(std::bind(&CC3DInterface::OnAddSkeletonLayer, this, std::placeholders::_1), this);
		InterfacePrivate->RenderModel.m_3DScene = InterfacePrivate->SceneMgr;

	}

	CC3DInterface::~CC3DInterface()
	{
		sigAddSkeletonLayer.unbindall();
	}

	void CC3DInterface::LoadModel(const std::string& szFile)
	{
		InterfacePrivate->SceneMgr->LoadMainModel((char*)szFile.c_str());
		InterfacePrivate->RenderModel.InitModelName(szFile);
		InterfacePrivate->HasPrepared = false;
	}

	void CC3DInterface::LoadZIP(const std::string& szFile)
	{
		const char* szXMLFile = "test.xml";

		std::wstring wPath = core::u8_ucs2(szFile);

		std::ifstream inStream(wPath, ios_base::in | ios_base::binary);
		std::stringstream buffer;
		buffer << inStream.rdbuf();
		std::string contents(buffer.str());

		HZIP hZip = nullptr;
		if (contents.empty())
		{
			hZip = OpenZip(szFile.c_str(), nullptr);
		}
		else
		{
			hZip = OpenZip((char*)contents.data(), contents.length(), nullptr);
		}

		if (hZip != NULL)
		{
			ZIPENTRY ze;
			int index;
			if (ZR_OK == FindZipItem(hZip, szXMLFile, true, &index, &ze))
			{
				char* pDataBuffer = new char[ze.unc_size];
				ZRESULT res = UnzipItem(hZip, index, pDataBuffer, ze.unc_size);
				if (res != ZR_OK)
				{
					delete[]pDataBuffer;
					CloseZip(hZip);
					return;
				}

				XMLResults xResults;
				XMLNode nodeModels = XMLNode::parseBuffer(pDataBuffer, ze.unc_size, "models", &xResults);
				delete[]pDataBuffer;
				//Get version and Version control
				const char* szVersion = nodeModels.getAttribute("version");
				if (szVersion != NULL)
				{
					double m_EffectVersion = atof(szVersion);
					if (m_EffectVersion > LIBVERSION_DOUBLE(LIBVERSION))
					{
						CloseZip(hZip);
						return;
					}
				}

				int i = -1;
				XMLNode nodeEffect = nodeModels.getChildNode("typeeffect", ++i);
				while (!nodeEffect.isEmpty())
				{
					std::string szType = nodeEffect.getAttribute("type");
					if (szType == "FacePBRModel")
					{
						InterfacePrivate->RenderModel.ReadConfig(nodeEffect, hZip, (char*)szFile.c_str());
						InterfacePrivate->HasPrepared = false;
						break;
					}

					nodeEffect = nodeModels.getChildNode("typeeffect", ++i);
				}
			}
		}
		CloseZip(hZip);
	}

	void CC3DInterface::SetHDRPath(const std::string& hdrPath)
	{
		InterfacePrivate->RenderModel.SetHDRPath(hdrPath);
		if (CC3DEnvironmentConfig::getInstance()->PbrRender)
		{
			CC3DEnvironmentConfig::getInstance()->PbrRender->SetHDRPath(hdrPath);
		}
	}

	void CC3DInterface::SetResourcePath(const std::string& resPath)
	{
		CC3DEnvironmentConfig::getInstance()->resourth_path = resPath;
		InterfacePrivate->ResourcePath = resPath;
	}

	void CC3DInterface::Play()
	{

		if (InterfacePrivate->StartTime < 0)
		{
			InterfacePrivate->StartTime = ccCurrentTime();
		}

		auto currentTime = ccCurrentTime();
		InterfacePrivate->RenderModel.RunTime = long(currentTime - InterfacePrivate->StartTime);

		float fSecond = InterfacePrivate->RenderModel.RunTime * 1.f / 1000;

		InterfacePrivate->SceneMgr->play(fSecond);
		

	}

	void CC3DInterface::SetScale(float Scale)
	{
		InterfacePrivate->Scale = Scale;
	}

	void CC3DInterface::SetTranslate(float x, float y)
	{
		InterfacePrivate->Translate = { x,y };
	}

	void CC3DInterface::SetRotate(float x, float y)
	{
		InterfacePrivate->Rotate = { x,y };
	}

	void CC3DInterface::SetInnerScale(float x, float y, float z)
	{
		InterfacePrivate->SceneMgr->m_Camera.SetScale(x, y, z);

		if (InterfacePrivate->SceneMgr->m_Model.size() > 0)
		{
			Vector3 Distanse = GetFirstModelBox().maxPoint - GetFirstModelBox().minPoint;

			float z = (std::max)(Distanse.x, (std::max)(Distanse.y, Distanse.z));
			auto campos = InterfacePrivate->SceneMgr->m_Camera.GetCameraPos();
			InterfacePrivate->SceneMgr->m_Project.SetFar(campos.z + 3 * z);
			InterfacePrivate->SceneMgr->m_Project.SetNear((std::max)(0.01f, campos.z - 3*z));
		}
	}

	void CC3DInterface::SetInnerTranslate(float x, float y, float z)
	{
		if (InterfacePrivate->SceneMgr->m_Model.size() > 0)
		{
			Vector3 tmp = GetFirstModelBox().maxPoint - GetFirstModelBox().minPoint;

			InterfacePrivate->SceneMgr->m_ModelControl.SetTranslate(x * tmp.x , y * tmp.y , z * tmp.z);
		}
		else
		{
			InterfacePrivate->SceneMgr->m_ModelControl.SetTranslate(x, y, z);
		}
	}

	void CC3DInterface::SetInnerRotate(float x, float y, float z)
	{
		if (InterfacePrivate->SceneMgr->m_Model.empty())
		{
			return;
		}
		InterfacePrivate->SceneMgr->m_ModelControl.SetRotate(x, y, z);

		if (InterfacePrivate->RenderModel.CheckIfHasDynamicBone())
		{
			auto& ParentMat = InterfacePrivate->SceneMgr->m_Skeleton[0]->m_RootNode[0]->ParentMat;

			ParentMat = InterfacePrivate->SceneMgr->m_ModelControl.m_ModelMatrix;
			InterfacePrivate->SceneMgr->m_Skeleton[0]->UpdateBone();

			InterfacePrivate->SceneMgr->m_ModelControl.m_ModelMatrix = InterfacePrivate->SceneMgr->m_ModelControl.m_ModelMatrix * glm::inverse(ParentMat);
		}
	}


	const BoundingBox& CC3DInterface::GetFirstModelBox() const
	{
		return InterfacePrivate->SceneMgr->m_Model[0]->m_ModelBox;
	}

	void CC3DInterface::OnAddSkeletonLayer(const std::map<std::string, int>& bone)
	{
		sigAddSkeletonLayer(bone);
	}

	void CC3DInterface::Render(int width, int height, std::shared_ptr<CC3DTextureRHI> RenderTargetTex)
	{
		InterfacePrivate->Size = { width,height };

		if (!InterfacePrivate->RenderTargetDBuffer || InterfacePrivate->RenderTargetDBuffer->GetWidth() != width || InterfacePrivate->RenderTargetDBuffer->GetHeight() != height)
		{
			InterfacePrivate->RenderTargetDBuffer = GetDynamicRHI()->CreateDoubleBuffer();
			if (GetDynamicRHI()->API == CC3DDynamicRHI::DX11)
			{
				DX11DoubleBufferWrapper::InitDoubleBuffer(InterfacePrivate->RenderTargetDBuffer, width, height, true, true, CC3DTextureRHI::SFT_A8R8G8B8);
			}
		}

		BaseRenderParam param;
		param.SetSize(width, height);
		param.SetDoubleBuffer(InterfacePrivate->RenderTargetDBuffer);

		SetInnerScale(InterfacePrivate->Scale, InterfacePrivate->Scale, InterfacePrivate->Scale);
		SetInnerTranslate(InterfacePrivate->Translate.x/ KSizeScale, -InterfacePrivate->Translate.y/ KSizeScale, 0.0);
		SetInnerRotate(InterfacePrivate->Rotate.y, InterfacePrivate->Rotate.x, 0.0);
		InterfacePrivate->Scale = 0;

		InterfacePrivate->SceneMgr->m_Project.SetAspect(width * 1.f / height);


		if (!InterfacePrivate->HasPrepared)
		{
			InterfacePrivate->HasPrepared = true;
			InterfacePrivate->RenderModel.Prepare(InterfacePrivate->ResourcePath);
			InterfacePrivate->RenderModel.RenderBground = true;
		}
	
		Play();

		InterfacePrivate->RenderModel.PreRender(param);

		auto pDoubleBuffer = InterfacePrivate->RenderTargetDBuffer;

		if (InterfacePrivate->RenderModel.GetDoubleBuffer() != NULL)
		{
			pDoubleBuffer = InterfacePrivate->RenderModel.GetDoubleBuffer();
		}

		pDoubleBuffer->BindDoubleBuffer();
		pDoubleBuffer->ClearA(0,0,0,1);
		pDoubleBuffer->ClearB(0,0,0,1);

		InterfacePrivate->RenderModel.Render(param);
		InterfacePrivate->RenderModel.PostRender(param);

		if (GetDynamicRHI()->API == CC3DDynamicRHI::DX11)
		{
			GetDynamicRHI()->ResolveSubresource(RenderTargetTex, InterfacePrivate->RenderTargetDBuffer->GetFBOTextureA());
		}
		else
		{
			RenderTargetTex->AttatchTextureId(InterfacePrivate->RenderTargetDBuffer->GetFBOTextureA());
		}

	}

	void CC3DInterface::LoadAnimateJson(const std::string& fileName)
	{
		if (InterfacePrivate&& InterfacePrivate->SceneMgr)
		{
			InterfacePrivate->SceneMgr->LoadAnimationJson(fileName);
		}
	}

	void CC3DInterface::SetRotateIBL(float x, float y)
	{
		InterfacePrivate->RenderModel.EffectConfig->ModelConfig.hdrRotateX = x;
		InterfacePrivate->RenderModel.EffectConfig->ModelConfig.hdrRotateY = y;
	}

	void CC3DInterface::SetHDRScale(float Scale)
	{
		InterfacePrivate->RenderModel.EffectConfig->ModelConfig.HDRScale = Scale;
	}

	void CC3DInterface::SetHDRContrast(float Contrast)
	{
		InterfacePrivate->RenderModel.EffectConfig->ModelConfig.HDRContrast = Contrast;
	}

	void CC3DInterface::SetMainLightInfo(const Vector3& Dir, const Vector3& Color, float LightStrength, float AmbientStrength)
	{
		InterfacePrivate->RenderModel.EffectConfig->ModelConfig.LightDir[0] = Dir.x;
		InterfacePrivate->RenderModel.EffectConfig->ModelConfig.LightDir[1] = Dir.y;
		InterfacePrivate->RenderModel.EffectConfig->ModelConfig.LightDir[2] = Dir.z;

		InterfacePrivate->RenderModel.EffectConfig->ModelConfig.LightColor[0] = Color.x;
		InterfacePrivate->RenderModel.EffectConfig->ModelConfig.LightColor[1] = Color.y;
		InterfacePrivate->RenderModel.EffectConfig->ModelConfig.LightColor[2] = Color.z;

		InterfacePrivate->RenderModel.EffectConfig->ModelConfig.AmbientStrength = AmbientStrength;
		InterfacePrivate->RenderModel.EffectConfig->ModelConfig.LightStrength = LightStrength;
	}

	void CC3DInterface::SetGamma(float gamma)
	{
		InterfacePrivate->RenderModel.EffectConfig->ModelConfig.Gamma = gamma;
	}

	void CC3DInterface::SetEnableDiffuseSRGB(bool enable)
	{
		InterfacePrivate->RenderModel.EffectConfig->ModelConfig.bEnableGammaRM = enable;
	}

	void CC3DInterface::SetFurData(const FurConfigData& data)
	{
		InterfacePrivate->RenderModel.EffectConfig->FurData.ConfigData = data;
	}

	void CC3DInterface::SetAOOffset(float offset)
	{
		InterfacePrivate->RenderModel.EffectConfig->ModelConfig.AOOffset = offset;
	}

	void CC3DInterface::SetEnableShadow(bool enable)
	{
		InterfacePrivate->RenderModel.EffectConfig->ModelConfig.bEnableShadow = enable;
	}

	void CC3DInterface::AddDynamicBone(const std::string& boneName)
	{
		{
			auto& SkeletonMgr = InterfacePrivate->SceneMgr->SkeletonManage;
			auto& DynamicBone = SkeletonMgr.CreateDefaultDynamicBone(boneName);
			InterfacePrivate->RenderModel.EffectConfig->ModelConfig.DyBone_array.push_back(DynamicBone);
			SkeletonMgr.AddDynamicBone(DynamicBone);
		}
		{
			if (CC3DEnvironmentConfig::getInstance()->SceneMgr)
			{
				auto& SkeletonMgr = CC3DEnvironmentConfig::getInstance()->SceneMgr->SkeletonManage;
				auto& DynamicBone = SkeletonMgr.CreateDefaultDynamicBone(boneName);
				InterfacePrivate->RenderModel.EffectConfig->ModelConfig.DyBone_array.push_back(DynamicBone);
				SkeletonMgr.AddDynamicBone(DynamicBone);
			}
		}
	}

	void CC3DInterface::UpdateDynamicParam(const CC3DImageFilter::dynamicBoneParameter& config)
	{
		auto& DyBoneArray = InterfacePrivate->RenderModel.EffectConfig->ModelConfig.DyBone_array;
		auto it = std::find_if(DyBoneArray.begin(), DyBoneArray.end(), [&config](const CC3DImageFilter::dynamicBoneParameter& item) 
			{return item.bone_name == config.bone_name; }
		);
		if (it != DyBoneArray.end())
		{
			(*it) = config;
		}
		
		InterfacePrivate->SceneMgr->SkeletonManage.UpdateDynamicBoneConfig(config);

		if (CC3DEnvironmentConfig::getInstance()->SceneMgr)
		{
			CC3DEnvironmentConfig::getInstance()->SceneMgr->SkeletonManage.UpdateDynamicBoneConfig(config);
		}
	}

	void CC3DInterface::DeleteDynamicBone(const std::string& db_name)
	{
		auto& DyBoneArray = InterfacePrivate->RenderModel.EffectConfig->ModelConfig.DyBone_array;
		auto it = std::find_if(DyBoneArray.begin(), DyBoneArray.end(), [&db_name](const CC3DImageFilter::dynamicBoneParameter& item)
			{return item.bone_name == db_name; }
		);
		if (it != DyBoneArray.end())
		{
			DyBoneArray.erase(it);
		}
		InterfacePrivate->SceneMgr->SkeletonManage.DeleteDynamicBone(db_name);

		if (CC3DEnvironmentConfig::getInstance()->SceneMgr)
		{
			CC3DEnvironmentConfig::getInstance()->SceneMgr->SkeletonManage.DeleteDynamicBone(db_name);
		}
	}

	void CC3DInterface::ResetDynamicBone()
	{
		InterfacePrivate->RenderModel.EffectConfig->ModelConfig.DyBone_array.clear();
		InterfacePrivate->SceneMgr->SkeletonManage.ResetDynamicBone();
	}

	void CC3DInterface::SetTonemappingType(const PBRToneMappingType& type)
	{
		InterfacePrivate->RenderModel.EffectConfig->ModelConfig.ToneMapping = type;
	}

	void CC3DInterface::SetBloomInfo(const BloomInfo& Bloom)
	{
		InterfacePrivate->RenderModel.EffectConfig->ModelConfig.Bloom = Bloom;
	}

	const ModelConfig& CC3DInterface::GetModeConfig() const
	{
		return InterfacePrivate->RenderModel.EffectConfig->ModelConfig;
	}


	const FurConfigData& CC3DInterface::GetFurData() const
	{
		return InterfacePrivate->RenderModel.EffectConfig->FurData.ConfigData;
	}

}