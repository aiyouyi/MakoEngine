#include "CRenderPBRModel.h"
#include "Toolbox/HeaderModelForCull.h"
#include "CC3DEngine/Scene/CC3DSceneManage.h"
#include "CC3DEngine/Render/CC3DPbrRender.h"
#include "Toolbox/FSObject.h"

#include "CC3DEngine/Render/CC3DPostProcessManager.h"
#include "CC3DEngine/FaceExpressDrive/CC3DExpressDrive.h"
#include "Common/CC3DEnvironmentConfig.h"
#include "EffectKernel/ShaderProgramManager.h"
#include "BaseDefine/json.hpp"

CRenderPBRModel::CRenderPBRModel()
{

	m_3DScene = NULL;
	m_Render = NULL;
	m_EnableWrite = true;
	m_pHdrData = NULL;
}

void CRenderPBRModel::Release()
{

	SAFE_DELETE(m_3DScene);
	SAFE_DELETE(m_Render);
	SAFE_DELETE_ARRAY(m_pHdrData);
	for (int i = 0; i < m_MatrialVideo.size(); i++)
	{
		SAFE_DELETE(m_MatrialVideo[i].Animation);
	}

	SAFE_DELETE(m_postManager);
	SAFE_DELETE(m_ExpressDrive);
	SAFE_DELETE(m_DoubleBuffer);
}

CRenderPBRModel::~CRenderPBRModel()
{
	Release();
}


bool CRenderPBRModel::ReadConfig(XMLNode & childNode, HZIP hZip, char * pFilePath)
{
	if (!childNode.isEmpty())
	{

		//毛发材质解析
		XMLNode nodeMaterial = childNode.getChildNode("FurMaterial", 0);
		if (!nodeMaterial.isEmpty())
		{
			const char* szFurName = nodeMaterial.getAttribute("name");
			if (szFurName != nullptr )
			{
				CC3DEnvironmentConfig::getInstance()->fur_material_name = szFurName;
			}

			const char* noise_name = nodeMaterial.getAttribute("NoiseTex");
			if (noise_name != nullptr)
			{
				noiseTex = GetDynamicRHI()->CreateTextureFromZip(hZip, noise_name, false);
			}

			const char* length_name = nodeMaterial.getAttribute("LengthTex");
			if (length_name)
			{
				lengthTex = GetDynamicRHI()->CreateTextureFromZip(hZip, length_name, false);
			}

			const char* furname = nodeMaterial.getAttribute("FurLength");
			if (furname != nullptr)
			{
				sscanf(furname, "%f", &fur_constbuffer.furLength);
			}

			const char* furlevel = nodeMaterial.getAttribute("FurLevel");
			if (furlevel != nullptr)
			{
				sscanf(furlevel, "%d", &numFurLayers);
			}

			const char* uvscale = nodeMaterial.getAttribute("UVScale");
			if (uvscale != nullptr)
			{
				sscanf(uvscale, "%f", &fur_constbuffer.uvScale);
			}


			const char* fgamma = nodeMaterial.getAttribute("gamma");
			if (fgamma != nullptr)
			{
				sscanf(fgamma, "%f", &fur_constbuffer.furGamma);
			}

			const char* useTM = nodeMaterial.getAttribute("useToneMapping");
			if (useTM != nullptr)
			{
				sscanf(useTM, "%d", &fur_constbuffer.useToneMapping);
			}

			const char* furColor = nodeMaterial.getAttribute("FurColor");
			if (furColor != nullptr)
			{
				sscanf(furColor, "%f,%f,%f", &fur_constbuffer.furColor.x, &fur_constbuffer.furColor.y, &fur_constbuffer.furColor.z);
			}

			const char* gravity = nodeMaterial.getAttribute("Gravity");
			if (gravity != nullptr)
			{
				sscanf(gravity, "%f,%f,%f", &fur_constbuffer.vForce.x, &fur_constbuffer.vForce.y, &fur_constbuffer.vForce.z);
			}

			const char* lightfilter = nodeMaterial.getAttribute("LightFilger");
			if (lightfilter != nullptr)
			{
				sscanf(lightfilter, "%f", &fur_constbuffer.lightFilter);
			}

			const char* lightexpo = nodeMaterial.getAttribute("FurLightExposure");
			if (lightexpo != nullptr)
			{
				sscanf(lightexpo, "%f", &fur_constbuffer.furLightExposure);
			}

			const char* fresnel = nodeMaterial.getAttribute("Fresnel");
			if (fresnel != nullptr)
			{
				sscanf(fresnel, "%f", &fur_constbuffer.fresnelLV);
			}

			const char* furmask = nodeMaterial.getAttribute("FurMask");
			if (furmask != nullptr)
			{
				sscanf(furmask, "%f", &fur_constbuffer.furMask);
			}

			const char* tming = nodeMaterial.getAttribute("Tming");
			if (tming != nullptr)
			{
				sscanf(tming, "%f", &fur_constbuffer.tming);
			}
			const char* ambient = nodeMaterial.getAttribute("ambientFur");
			if (ambient != nullptr)
			{
				sscanf(ambient, "%f", &ambientFur);
			}
		}

		//model
		XMLNode nodeGltfModel = childNode.getChildNode("gltf", 0);
		if (!nodeGltfModel.isEmpty())
		{
			const char *szEnableWrite = nodeGltfModel.getAttribute("enablewrite");
			if (szEnableWrite != NULL && !strcmp(szEnableWrite, "false"))
			{
				m_EnableWrite = false;
			}

			const char *camIndex = nodeGltfModel.getAttribute("camIndex");
			if (camIndex != NULL )
			{
				sscanf(camIndex, "%d", &m_nCamIndex);
			}

			const char *matScale = nodeGltfModel.getAttribute("matScale");
			if (matScale != NULL)
			{
				sscanf(matScale, "%f", &m_MatScale);
			}

			const char *matHead = nodeGltfModel.getAttribute("matScaleHead");
			if (matHead != NULL)
			{
				sscanf(matHead, "%f", &m_MatScaleHead);
			}
			const char *rotateX = nodeGltfModel.getAttribute("rotateX");
			if (rotateX != NULL)
			{
				sscanf(rotateX, "%f", &m_rotateX);
			}

			const char *szHdrName = nodeGltfModel.getAttribute("hdr");
			if (szHdrName != NULL)
			{
				m_pHdrData = DXUtils::CreateHDRFromZIP(hZip, szHdrName, hdrWidth, hdrHeight);
			}

			const char* szHairShift = nodeGltfModel.getAttribute("hair_shift");
			if (szHairShift)
			{
				hairShiftTex = GetDynamicRHI()->CreateTextureFromZip(hZip, szHairShift);
			}

			const char* szHairOutLineMask = nodeGltfModel.getAttribute("OutlineMask");
			if (szHairOutLineMask)
			{
				hairOutLineMaskTex = GetDynamicRHI()->CreateTextureFromZip(hZip, szHairOutLineMask);
			}

			const char *szModelName = nodeGltfModel.getAttribute("model");
			if (szModelName != NULL)
			{
				int index;
				ZIPENTRY ze;

				if (FindZipItem(hZip, szModelName, true, &index, &ze) == ZR_OK)
				{
					char *pDataBuffer = new char[ze.unc_size];
					ZRESULT res = UnzipItem(hZip, index, pDataBuffer, ze.unc_size);
					if (res == ZR_OK)
					{
						if (m_3DScene == NULL)
						{
							m_3DScene = new CC3DSceneManage();
						}	
						m_3DScene->LoadMainModel((char*)szModelName, (const unsigned char*)pDataBuffer, ze.unc_size);
					}
					SAFE_DELETE_ARRAY(pDataBuffer);
				}
			}

			const char *szAnimation = nodeGltfModel.getAttribute("AnimationType");
			if (szAnimation != NULL)
			{
				sscanf(szAnimation, "%d", &m_AnimationType);
			}

			const char *szInfo = nodeGltfModel.getAttribute("LightDir");
			if (szInfo != NULL)
			{
				sscanf(szInfo, "%f,%f,%f,%f", LightDir, LightDir+1, LightDir+2, LightDir+3);
			}
			szInfo = nodeGltfModel.getAttribute("LightColor");
			if (szInfo != NULL)
			{
				sscanf(szInfo, "%f,%f,%f", LightColor, LightColor + 1, LightColor + 2);
			}
			szInfo = nodeGltfModel.getAttribute("ambientStrength");
			if (szInfo != NULL)
			{
				sscanf(szInfo, "%f", &ambientStrength);
			}

			szInfo = nodeGltfModel.getAttribute("RoughnessRate");
			if (szInfo != NULL)
			{
				sscanf(szInfo, "%f", &RoughnessRate);
			}

			szInfo = nodeGltfModel.getAttribute("AnimateRate");
			if (szInfo != NULL)
			{
				sscanf(szInfo, "%f", &AnimateRate);
			}
			szInfo = nodeGltfModel.getAttribute("materialType");
			if (szInfo != NULL)
			{
				sscanf(szInfo, "%d", &materialType);
			}

			szInfo  = nodeGltfModel.getAttribute("SwitchPalate");
			if (szInfo != NULL)
			{
				m_SwitchPalate = szInfo;
			}


			szInfo = nodeGltfModel.getAttribute("rotateIBL");
			if (szInfo != NULL)
			{
				sscanf(szInfo, "%f", &m_RotateIBL);
			}

			szInfo = nodeGltfModel.getAttribute("EnableShadow");
			if (szInfo != NULL)
			{
				sscanf(szInfo, "%d", &m_EnableShadow);
			}

			szInfo = nodeGltfModel.getAttribute("gamma");
			if (szInfo != NULL)
			{
				sscanf(szInfo, "%f", &gamma);
			}

			szInfo = nodeGltfModel.getAttribute("gammaRM");
			if (szInfo != NULL)
			{
				sscanf(szInfo, "%d", &gammaRM);
			}

			szInfo = nodeGltfModel.getAttribute("EnableExpress");
			if (szInfo != NULL)
			{
				int express = 0;
				sscanf(szInfo, "%d", &express);
				if (express)
				{
					m_ExpressDrive = new CC3DExpressDrive();
				}

			}

			szInfo = nodeGltfModel.getAttribute("EnableOutLine");
			if (szInfo != NULL)
			{
				sscanf(szInfo, "%d", &m_EnableOutLine);
			}
			

			{
				float NormalIntensity = 0.0;
				float FrontNoramlScale = 1.0f;
				float FrontNormalOffset = 0.0f;
				float PrimaryShift = 0.2f;
				float SecondaryShift = 0.2f;
				float SpecularPower = 100.f;
				float ShiftU = 1.0f;
				float KajiyaSpecularScale = 0.5f;
				float KajiyaSpecularWidth = 0.5f;

				nlohmann::json Root;

				szInfo = nodeGltfModel.getAttribute("FrontNoramlScale");
				if (szInfo != NULL)
				{
					sscanf(szInfo, "%0.2f", &FrontNoramlScale);
				}

				szInfo = nodeGltfModel.getAttribute("FrontNormalOffset");
				if (szInfo != NULL)
				{
					sscanf(szInfo, "%0.2f", &FrontNormalOffset);
				}

				szInfo = nodeGltfModel.getAttribute("NormalIntensity");
				if (szInfo != NULL)
				{
					sscanf(szInfo, "%0.2f", &NormalIntensity);
				}

				Root["Normal"] = { FrontNoramlScale,FrontNormalOffset,NormalIntensity };

				szInfo = nodeGltfModel.getAttribute("PrimaryShift");
				if (szInfo != NULL)
				{
					sscanf(szInfo, "%0.2f", &PrimaryShift);
				}

				szInfo = nodeGltfModel.getAttribute("SecondaryShift");
				if (szInfo != NULL)
				{
					sscanf(szInfo, "%0.2f", &SecondaryShift);
				}

				szInfo = nodeGltfModel.getAttribute("SpecularPower");
				if (szInfo != NULL)
				{
					sscanf(szInfo, "%0.2f", &SpecularPower);
				}

				szInfo = nodeGltfModel.getAttribute("ShiftU");
				if (szInfo != NULL)
				{
					sscanf(szInfo, "%0.2f", &ShiftU);
				}

				szInfo = nodeGltfModel.getAttribute("KajiyaSpecularScale");
				if (szInfo != NULL)
				{
					sscanf(szInfo, "%0.2f", &KajiyaSpecularScale);
				}

				szInfo = nodeGltfModel.getAttribute("KajiyaSpecularWidth");
				if (szInfo != NULL)
				{
					sscanf(szInfo, "%0.2f", &KajiyaSpecularWidth);
				}

				Root["Kajiya"] = { PrimaryShift ,SecondaryShift ,SpecularPower ,ShiftU ,KajiyaSpecularScale ,KajiyaSpecularWidth };
				
				Vector4 HairOutLineColor;
				szInfo = nodeGltfModel.getAttribute("HairOutLineColor");
				if (szInfo != NULL)
				{
					sscanf(szInfo, "%f,%f,%f,%f", &HairOutLineColor.x, &HairOutLineColor.y, &HairOutLineColor.z, &HairOutLineColor.w);
				}
				Root["HairOutLineColor"] = { HairOutLineColor.x,HairOutLineColor.y,HairOutLineColor.z,HairOutLineColor.w };

				Vector4 BodyOutlineColor;
				szInfo = nodeGltfModel.getAttribute("BodyOutlineColor");
				if (szInfo != NULL)
				{
					sscanf(szInfo, "%f,%f,%f,%f", &BodyOutlineColor.x, &BodyOutlineColor.y, &BodyOutlineColor.z, &BodyOutlineColor.w);
				}
				Root["BodyOutlineColor"] = { BodyOutlineColor.x,BodyOutlineColor.y,BodyOutlineColor.z,HairOutLineColor.w };

				Vector4 DefOutlineColor;
				szInfo = nodeGltfModel.getAttribute("DefOutlineColor");
				if (szInfo != NULL)
				{
					sscanf(szInfo, "%f,%f,%f,%f", &DefOutlineColor.x, &DefOutlineColor.y, &DefOutlineColor.z, &DefOutlineColor.w);
				}
				Root["DefOutlineColor"] = { DefOutlineColor.x,DefOutlineColor.y,DefOutlineColor.z,DefOutlineColor.w };

				m_RenderMaterialParams = Root.dump();
			}

		}
		XMLNode nodePost = childNode.getChildNode("Post", 0);
		if (!nodePost.isEmpty())
		{
			m_postManager = new PostProcessManager();
			const char *szInfo = nodePost.getAttribute("bloom");
			if (szInfo != NULL)
			{
				sscanf(szInfo, "%f,%f,%d" , &m_bloomAlpha, &m_bloomradius, &m_Bloomlooptime);
			}
			szInfo = nodePost.getAttribute("bloomBegin");
			if (szInfo != NULL)
			{
				sscanf(szInfo, "%f", &m_bloomBeginAlpha);
			}
		}

		int i = -1;
		XMLNode nodeDyBone = childNode.getChildNode("DyBone", ++i);
		while (!nodeDyBone.isEmpty())
		{
			dynamicBoneParameter Param;
			const char *szName = nodeDyBone.getAttribute("name");
			if (szName != NULL)
			{
				Param.bone_name = szName;
			}
			const char * szInfo = nodeDyBone.getAttribute("Damping");
			if (szInfo != NULL)
			{
				sscanf(szInfo, "%f", &Param._fDamping);
			}
			szInfo = nodeDyBone.getAttribute("Elasticity");
			if (szInfo != NULL)
			{
				sscanf(szInfo, "%f", &Param._fElasticity);
			}

			szInfo = nodeDyBone.getAttribute("Stiffness");
			if (szInfo != NULL)
			{
				sscanf(szInfo, "%f", &Param._fStiffness);
			}
			szInfo = nodeDyBone.getAttribute("Inert");
			if (szInfo != NULL)
			{
				sscanf(szInfo, "%f", &Param._fInert);
			}
			szInfo = nodeDyBone.getAttribute("Radius");
			if (szInfo != NULL)
			{
				sscanf(szInfo, "%f", &Param._fRadius);
			}
			szInfo = nodeDyBone.getAttribute("EndLength");
			if (szInfo != NULL)
			{
				sscanf(szInfo, "%f", &Param._fEndLength);
			}
			m_DyBone_array.push_back(Param);
			nodeDyBone = childNode.getChildNode("DyBone", ++i);
		}

		if (m_3DScene!=NULL&&m_3DScene->m_Skeleton.size() > 0 && m_3DScene->m_Skeleton[0]->m_RootNode.size() > 0 && m_3DScene->m_Model[0]->m_hasSkin)
		{
			m_3DScene->m_Skeleton[0]->AddDynamicBone(m_DyBone_array);
		}
		//单张贴图
		i = -1;
		XMLNode nodeDrawable = childNode.getChildNode("drawable", ++i);
		char szFullFile[256];
		while (!nodeDrawable.isEmpty())
		{
			MaterialInfo mv;
			mv.MaterialType = "Effect";
			
			const char *szDrawableName = nodeDrawable.getAttribute("name");
			mv.MaterialName = szDrawableName;
			
			const char *szImagePath = nodeDrawable.getAttribute("image");

			sprintf(szFullFile, "%s/%s", pFilePath, szImagePath);

			const char *szGenMipmap = nodeDrawable.getAttribute("genMipmaps");
			bool bGenMipmap = false;
			if (szGenMipmap != NULL && (strcmp(szGenMipmap, "yes") || strcmp(szGenMipmap, "YES")))
			{
				bGenMipmap = true;
			}

			std::shared_ptr<CC3DTextureRHI> TexRHI = GetDynamicRHI()->FetchTexture(szFullFile, bGenMipmap);
			if (!TexRHI)
			{
				TexRHI = GetDynamicRHI()->CreateTextureFromZip(hZip, szImagePath, bGenMipmap);
				GetDynamicRHI()->RecoredTexture(szFullFile, TexRHI);
				mv.Animation = new BitmapDrawable(TexRHI);
			}

			m_MatrialVideo.push_back(mv);
			nodeDrawable = childNode.getChildNode("drawable", ++i);
		}
		//序列帧
		i = -1;
		nodeDrawable = childNode.getChildNode("anidrawable", ++i);
		while (!nodeDrawable.isEmpty())
		{	
			MaterialInfo mv;
			mv.MaterialType = "Effect";
			if (!nodeDrawable.isEmpty()) {
				const char* szDrawableName = nodeDrawable.getAttribute("name");
				const char* szDrawabletype = nodeDrawable.getAttribute("type");
				mv.MaterialName = szDrawableName;
				AnimationDrawable* drawable = new AnimationDrawable();
				if (szDrawabletype!=NULL)
				{
					mv.MaterialType = szDrawabletype;
				}

				const char* szOffset = nodeDrawable.getAttribute("offset");
				long offset = atol(szOffset);
				drawable->setOffset(offset);

				const char* szLoopMode = nodeDrawable.getAttribute("loopMode");
				if (szLoopMode != NULL && strcmp(szLoopMode, "oneShot") == 0)
					drawable->setLoopMode(ELM_ONESHOT);
				else
					drawable->setLoopMode(ELM_REPEAT);

				const char* szGenMipmap = nodeDrawable.getAttribute("genMipmaps");
				bool bGenMipmap = false;
				if (szGenMipmap != NULL && (strcmp(szGenMipmap, "yes") || strcmp(szGenMipmap, "YES")))
					bGenMipmap = true;

				const char *szItems = nodeDrawable.getAttribute("items");
				const char *szItemInfo = nodeDrawable.getAttribute("iteminfo");
				if (szItems != NULL && szItemInfo != NULL)
				{
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

							std::shared_ptr<CC3DTextureRHI> pTex = GetDynamicRHI()->FetchTexture(szFullFile, false);
							if (pTex == NULL)
							{
								pTex = GetDynamicRHI()->CreateTextureFromZip(hZip, szImagePath, bGenMipmap);
								GetDynamicRHI()->RecoredTexture(szFullFile, pTex);
							}

							long during = nDuring;

							drawable->appandTex(during, pTex);
						}
					}
					mv.Animation = drawable;
				}

				m_MatrialVideo.push_back(mv);
				nodeDrawable = childNode.getChildNode("anidrawable", ++i);
			}
		}

		return true;
	}
	return false;
}

bool CRenderPBRModel::Prepare(std::string &resourePath)
{

	if (m_3DScene == NULL || m_3DScene->m_Model.size()==0)
	{
		return false;
	}
	m_Render = new CC3DPbrRender();
	m_Render->InitSecne(m_3DScene);
	m_Render->SetShaderResource(resourePath);
	m_Render->m_RenderCube->setHDRData(m_pHdrData, hdrWidth, hdrHeight);
	m_Render->m_EnableWrite = m_EnableWrite;
	m_Render->m_RotateX = m_RotateIBL;
	m_Render->SetEnableShadow(m_EnableShadow == 1);
	m_Render->SetEnableOutline(m_EnableOutLine == 1);
	m_Render->SetRenderParams(m_RenderMaterialParams);

	if (!DefOutLineMaskTex)
	{
		DefOutLineMaskTex = GetDynamicRHI()->CreateTexture(1, 1, 1, 1);
	}
	m_Render->SetHairOutlineMask(hairOutLineMaskTex, DefOutLineMaskTex);

	if (m_postManager!=NULL)
	{
		m_postManager->SetShaderResource(resourePath);
		CCVetexAttribute pAttribute[] =
		{
			{VERTEX_ATTRIB_POSITION, FLOAT_C3},
		};

		if (pLDR2HDR == nullptr)
		{
			std::string fsPath = resourePath + "/Shader/3D/LDR2HDR.fx";
			pLDR2HDR = ShaderProgramManager::GetInstance()->GetOrCreateShaderByPathAndAttribs(fsPath, pAttribute, 1);
		}

		if (pHDR2LDR == nullptr)
		{
			std::string fsPath = resourePath + "/Shader/3D/HDR2LDR.fx";
			pHDR2LDR = ShaderProgramManager::GetInstance()->GetOrCreateShaderByPathAndAttribs(fsPath, pAttribute, 1);
		}
		Vector3 arrCoords[4] = { Vector3(-1, -1, 1), Vector3(1, -1, 1), Vector3(-1, 1, 1), Vector3(1, 1, 1) };
		unsigned short index[] =
		{
			0, 1, 2,
			1, 2, 3
		};
		mVertexBuffer = GetDynamicRHI()->CreateVertexBuffer((float*)arrCoords, 4, 3);
		mIndexBuffer = GetDynamicRHI()->CreateIndexBuffer(index, 2);
	}

	if (m_ExpressDrive!=NULL)
	{
		auto path1 = resourePath + "/3DModels/model_74.endat";
		m_ExpressDrive->SetModelPath(path1.c_str());
	}
	lastTimeStamp = std::chrono::system_clock::now();


	return true;
}

void CRenderPBRModel::PreRender(BaseRenderParam & RenderParam)
{
	if (m_3DScene == NULL || m_3DScene->m_Model.size() == 0)
	{
		return;
	}

	m_Render->RenderCube();

	int width = RenderParam.GetWidth();
	int height = RenderParam.GetHeight();
	int nFaceCount = RenderParam.GetFaceCount();
	int EnableEmiss = 0;
	if (m_postManager != NULL)
	{
		if (m_DoubleBuffer == NULL || m_DoubleBuffer->GetWidth() != width || m_DoubleBuffer->GetHeight() != height)
		{
			SAFE_DELETE(m_DoubleBuffer);
			m_DoubleBuffer = new DX11DoubleBuffer();
			m_DoubleBuffer->InitDoubleBuffer(width, height,true,true, DXGI_FORMAT_R16G16B16A16_FLOAT);
		}

		m_DoubleBuffer->BindFBOA();
		pLDR2HDR->useShader();
		RenderParam.GetDoubleBuffer()->SetAShaderResource(0);
		GetDynamicRHI()->DrawPrimitive(mVertexBuffer, mIndexBuffer);
		EnableEmiss = 1;
	}


	if (nFaceCount > 0)
	{
		if (m_ExpressDrive != NULL && RenderParam.m_pBGRA_Src != NULL)
		{
			m_ExpressDrive->GetBlendshapeWeights(RenderParam.GetBGR_SRC(), width, height, RenderParam.GetFacePoint(0));
			std::vector<float> coeffs = m_ExpressDrive->GetCoeffs();
			if (coeffs.size() < BlendShapeCoffes)
			{
				return;
			}
			auto &BlendShapeName = m_ExpressDrive->GetBlendShapeNames();
			std::vector<float> headRotation = m_ExpressDrive->GetHeadRotation();

			int nModel = m_3DScene->m_Model.size();
			for (int i = 0; i < nModel; i++)
			{
				CC3DModel *pModel = m_3DScene->m_Model[i];
				int nMesh = pModel->m_ModelMesh.size();
				for (int j = 0; j < nMesh; j++)
				{
					CC3DMesh *pMesh = pModel->m_ModelMesh[j];
					if (pMesh->m_pBlendShapeName.size() > 0)
					{
						pMesh->GenVertWithCoeffs(coeffs, BlendShapeName);
					}
				}
			}
		}

	}

	m_3DScene->m_nSelectCamera = m_nCamIndex;
	float fSecond = m_runTime * 1.f / 1000;
	CC3DDirectionalLight *light1 = (CC3DDirectionalLight *)m_3DScene->m_Light[0];
	light1->SetLightDirection(glm::vec3(LightDir[0], LightDir[1], LightDir[2]));
	light1->SetLightColor(LightColor[0], LightColor[1], LightColor[2]);
	//light1->SetLightDirection(glm::vec3(0.0, -1.0, 1.0));
	m_3DScene->m_Light[0]->SetLightDiffuseIntensity(LightDir[3]);

	if (m_AnimationType == 0)
	{
		m_3DScene->play(fSecond*AnimateRate);

	}
	else if (m_AnimationType == 1)
	{
		m_3DScene->playOnce(fSecond*AnimateRate);
	}
	auto &modelMaterial = m_3DScene->m_Model[0]->m_ModelMaterial;
	for (int i = 0; i < modelMaterial.size(); i++)
	{
		modelMaterial[i]->SetParameter("roughnessRate", &RoughnessRate, 0, sizeof(float));
		modelMaterial[i]->SetParameter("reverseY", &ReverseY, 0, sizeof(int));
		modelMaterial[i]->SetParameter("ambientStrength", &ambientStrength, 0, sizeof(float));
		modelMaterial[i]->SetParameter("u_EnbleRMGamma", &gammaRM, 0, sizeof(int));
		modelMaterial[i]->SetParameter("gamma", &gamma, 0, sizeof(float));
		modelMaterial[i]->SetParameter("u_EnbleEmiss", &EnableEmiss, 0,sizeof(int));

		
		if (modelMaterial[i]->GET_CONSTBUFFER(PBRConstantBuffer).EnableKajiya == 1)
		{
			modelMaterial[i]->SetTexture2D("ShiftTex", hairShiftTex);
		}

		if (modelMaterial[i]->MaterialName == CC3DEnvironmentConfig::getInstance()->fur_material_name)
		{
			if (ambientFur>-1)
			{
				modelMaterial[i]->SetParameter("ambientStrength", &ambientFur, 0, sizeof(float));
			}
			
			auto furMaterial = dynamic_cast<CC3DFurMaterial*>(modelMaterial[i]);
			if (furMaterial != nullptr)
			{
				furMaterial->fur_constBuffer = fur_constbuffer;
				furMaterial->numLayers = numFurLayers;
				furMaterial->noiseTexture = noiseTex;
				furMaterial->lengthTexture = lengthTex;
				furMaterial->furLength = fur_constbuffer.furLength;
				furMaterial->UVScale = fur_constbuffer.uvScale;
			}
		}
	}
	for (int i = 0; i < m_MatrialVideo.size(); i++)
	{

		for (int j = 0; j < modelMaterial.size(); j++)
		{
			if (m_MatrialVideo[i].MaterialName == modelMaterial[j]->MaterialName)
			{
				if (m_MatrialVideo[i].MaterialType=="emiss")
				{
					modelMaterial[j]->m_EmissiveTexture = m_MatrialVideo[i].Animation->GetTex(m_runTime);
				}
				else
				{
					modelMaterial[j]->m_BaseColorTexture = m_MatrialVideo[i].Animation->GetTex(m_runTime);
				}

				break;
			}
		}
	}

	{
		auto end = std::chrono::system_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - lastTimeStamp);
		CC3DEnvironmentConfig::getInstance()->deltaTime = double(duration.count()) * std::chrono::microseconds::period::num / std::chrono::microseconds::period::den;
		lastTimeStamp = end;
	}
	if (m_EnableShadow == 1)
	{
		m_3DScene->m_ModelControl.UpdateModelMatrix();
		m_Render->RenderShadowMap();
	}
}

void CRenderPBRModel::Render(BaseRenderParam & RenderParam)
{
	if (m_3DScene == NULL || m_3DScene->m_Model.size() == 0)
	{
		return;
	}
	int width = RenderParam.GetWidth();
	int height = RenderParam.GetHeight();



// 	if (m_DyBone_array.size() > 0 && m_3DScene->m_Skeleton.size() > 0 && m_3DScene->m_Skeleton[0]->m_RootNode.size() > 0 && m_3DScene->m_Model[0]->m_hasSkin)
// 	{
// 		m_3DScene->m_Skeleton[0]->UpdateBone();
// 		m_3DScene->m_ModelControl.m_ModelMatrix = m_3DScene->m_ModelControl.m_ModelMatrix * glm::inverse(m_3DScene->m_Skeleton[0]->m_RootNode[0]->ParentMat);
// 	}
	
	m_Render->Render(width, height);

}

void CRenderPBRModel::PostRender(BaseRenderParam & RenderParam)
{
	if (m_3DScene == NULL || m_3DScene->m_Model.size() == 0)
	{
		return;
	}
	int width = RenderParam.GetWidth();
	int height = RenderParam.GetHeight();
	auto pDoubleBuffer = RenderParam.GetDoubleBuffer();
	if (m_postManager != NULL)
	{
		m_postManager->Init(width, height);
		m_postManager->m_FBO_Ext = m_DoubleBuffer;
		m_postManager->m_bloomRadius = m_bloomradius*0.5;

		m_postManager->m_bloomAlpha = m_bloomAlpha;
		if (m_Bloomlooptime > 0)
		{
			int looptime = m_Bloomlooptime * 2;

			float alpha = (m_runTime % looptime)*2.f / looptime;
			if (alpha > 1)
			{
				alpha = 1;
			}
			m_postManager->m_bloomAlpha = (m_bloomAlpha - m_bloomBeginAlpha)*alpha + m_bloomBeginAlpha;

		}

		m_postManager->_enableBloom = true;
		m_postManager->Process();

		pDoubleBuffer->BindFBOA();
		pHDR2LDR->useShader();
		m_DoubleBuffer->SetAShaderResource(0);
		GetDynamicRHI()->DrawPrimitive(mVertexBuffer, mIndexBuffer);
	}

}
