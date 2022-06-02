#include "FacialAnimation.h"
#include "Toolbox/xmlParser.h"
#include "Toolbox/Helper.h"
#include "Toolbox/Render/TextureRHI.h"
#include "Toolbox/Render/DynamicRHI.h"
//#include<vld.h>


extern DX11Texture *CreateFromZIP(HZIP hZip, const char *szImagePath, bool bGenMipmap);

bool loadFSObject(HZIP hZip, const char *szFile, FSObject *pObject, XMLNode &nodeCocosModel)
{
	const char *szPosition = nodeCocosModel.getAttribute("postion");
	Vec3 vPos(0, 0, 0);
	if (szPosition != NULL)
	{
		sscanf(szPosition, "%f,%f,%f", &vPos.x, &vPos.y, &vPos.z);
	}

	const char *szScale = nodeCocosModel.getAttribute("scale");
	Vec3 vScale(1, 1, 1);
	if (szPosition != NULL)
	{
		sscanf(szScale, "%f,%f,%f", &vScale.x, &vScale.y, &vScale.z);
	}

	const char *szRotate = nodeCocosModel.getAttribute("rotate");
	Vec3 vRotate(0, 0, 0);
	if (szRotate != NULL)
	{
		sscanf(szRotate, "%f,%f,%f", &vRotate.x, &vRotate.y, &vRotate.z);
	}

	pObject->setPosition3D(vPos);
	pObject->setRotation3D(vRotate);
	pObject->setScaleX(vScale.x);
	pObject->setScaleY(vScale.x);
	pObject->setScaleZ(vScale.z);


	const char *szDefaultAnimation = nodeCocosModel.getAttribute("defaultAnimation");
	if (szDefaultAnimation != NULL)
	{
		const char *szAnimationFile = nodeCocosModel.getAttribute("file");
		cocos2d::Animation3D *pAnimation = cocos2d::Animation3D::createFromZip(szAnimationFile, hZip, szFile, szDefaultAnimation);
		cocos2d::Animate3D *_meshAnimate = cocos2d::Animate3D::create(pAnimation);
		cocos2d::RepeatForever *pRepeatAnimation = cocos2d::RepeatForever::create(_meshAnimate);
		if (pRepeatAnimation != NULL)
		{
			pObject->addAnimation(szDefaultAnimation, pRepeatAnimation);
			pObject->play(szDefaultAnimation);
		}
	}
	else
	{
		XMLNode animationNode = nodeCocosModel.getChildNode("animation", 0);
		if (!animationNode.isEmpty())
		{
			const char *animationFile = animationNode.getAttribute("file");

			if (animationFile != NULL)
			{
				const char *id = animationNode.getAttribute("id");
				if (id == NULL)
				{
					id = "Take 001";
				}
				//load animation
				cocos2d::Animation3D *pAnimation = cocos2d::Animation3D::createFromZip(animationFile, hZip, szFile, id);
				map<string, cocos2d::Action *> mapAnimates;
				int iClip = -1;
				XMLNode animationClip = animationNode.getChildNode("animationClip", ++iClip);
				while (!animationClip.isEmpty())
				{
					const char *id = animationClip.getAttribute("id");
					const char *szStart = animationClip.getAttribute("start");
					const char *szEnd = animationClip.getAttribute("end");
					const char *szFps = animationClip.getAttribute("fps");
					const char *szRepeatMode = animationClip.getAttribute("repeatmode");
					int start = 0;
					int end = 0;
					int fps = 30;
					if (szStart != NULL)
					{
						start = atoi(szStart);
					}
					if (szEnd != NULL)
					{
						end = atoi(szEnd);
					}
					if (szFps != NULL)
					{
						fps = atoi(szFps);
					}
					if (szRepeatMode == NULL)
					{
						szRepeatMode = "repeat";
					}

					cocos2d::Animate3D *_meshAnimate = cocos2d::Animate3D::createWithFrames(pAnimation, start, end, fps);

					if (strcmp(szRepeatMode, "repeat") == 0)
					{
						cocos2d::RepeatForever *pRepeatAnimation = cocos2d::RepeatForever::create(_meshAnimate);
						if (pRepeatAnimation != NULL)
						{
							mapAnimates.insert(make_pair(id, pRepeatAnimation));
						}
					}
					else
					{
						mapAnimates.insert(make_pair(id, _meshAnimate));
					}

					animationClip = animationNode.getChildNode("animationClip", ++iClip);
				}

				if (mapAnimates.size() == 0)
				{
					cocos2d::Animate3D *_meshAnimate = cocos2d::Animate3D::create(pAnimation);
					cocos2d::RepeatForever *pRepeatAnimation = cocos2d::RepeatForever::create(_meshAnimate);
					if (pRepeatAnimation != NULL)
					{
						mapAnimates.insert(make_pair(id, pRepeatAnimation));
					}
				}

				map<string, cocos2d::Action *>::iterator it = mapAnimates.begin();
				while (it != mapAnimates.end())
				{
					pObject->addAnimation(it->first, it->second);

					++it;
				}
			}
		}

		const char *szDefaultAnimationClip = nodeCocosModel.getAttribute("defaultAnimationClip");
		if (szDefaultAnimationClip != NULL)
		{
			pObject->play(szDefaultAnimationClip);
		}
	}

	int i = -1;
	XMLNode childNode = nodeCocosModel.getChildNode("fsObject", ++i);
	while (!childNode.isEmpty())
	{
		const char *szModelPath = childNode.getAttribute("file");
		FSObject *pChild = FSObject::createFromZip(szModelPath, hZip, szFile);
		if (pChild != NULL)
		{
			loadFSObject(hZip, szFile, pChild, childNode);
		}

		pObject->addChild(pChild);
		childNode = nodeCocosModel.getChildNode("fsObject", ++i);
	}

	i = -1;
	childNode = nodeCocosModel.getChildNode("facialObject", ++i);
	while (!childNode.isEmpty())
	{
		const char *szModelPath = childNode.getAttribute("file");
		const char *szNeckBone = childNode.getAttribute("neckBone");
		const char *szNeckBone2 = childNode.getAttribute("neckBone2");
		FacialObject *pChild = FacialObject::createFromZip(szModelPath, hZip, szFile);
		if (pChild != NULL)
		{
			loadFSObject(hZip, szFile, pChild, childNode);

			if (szNeckBone != NULL)
			{
				pChild->setNeckBone(szNeckBone, szNeckBone2);
			}
		}

		pObject->addChild(pChild);
		childNode = nodeCocosModel.getChildNode("facialObject", ++i);
	}

	return pObject;
}


bool SceneInfo::loadScene(const char *szFile)
{
	destory();

	m_zip_file = string(szFile);
	char szFullFile[256];
	HZIP hZip = OpenZip(szFile, NULL);
	if (hZip != NULL)
	{
		int index;
		ZIPENTRY ze;
		long effectDuring = -1;
		char *szScene = "scene.xml";

		if (ZR_OK == FindZipItem(hZip, szScene, true, &index, &ze))
		{
			char *pDataBuffer = new char[ze.unc_size];
			ZRESULT res = UnzipItem(hZip, index, pDataBuffer, ze.unc_size);
			if (res != ZR_OK)
			{
				delete[]pDataBuffer;
				CloseZip(hZip);

				return 0;
			}

			//解析xml
			XMLResults xResults;
			XMLNode nodeModels = XMLNode::parseBuffer(pDataBuffer, ze.unc_size, "scene", &xResults);
			delete[]pDataBuffer;

			//载入帧动画元素
			typedef map<string, Drawable *> DrawableMap_;
			DrawableMap_ m_mapImage;
			int i = -1;
			XMLNode nodeDrawable = nodeModels.getChildNode("drawable", ++i);
			while (!nodeDrawable.isEmpty())
			{
				const char *szDrawableName = nodeDrawable.getAttribute("name");
				const char *szImagePath = nodeDrawable.getAttribute("image");

				sprintf(szFullFile, "%s/%s", szFile, szImagePath);

				const char *szGenMipmap = nodeDrawable.getAttribute("genMipmaps");
				bool bGenMipmap = false;
				if (szGenMipmap != NULL && (strcmp(szGenMipmap, "yes") || strcmp(szGenMipmap, "YES")))
				{
					bGenMipmap = true;
				}

				//防止重复加载图片

				std::shared_ptr< CC3DTextureRHI> TexRHI = GetDynamicRHI()->FetchTexture(szFullFile, bGenMipmap);
				if (TexRHI == nullptr)
				{
					TexRHI = GetDynamicRHI()->CreateTextureFromZip(hZip, szImagePath, bGenMipmap);
					GetDynamicRHI()->RecoredTexture(szFullFile, TexRHI);
				}

				m_mapImage.insert(make_pair(szDrawableName, new BitmapDrawable(TexRHI)));


				nodeDrawable = nodeModels.getChildNode("drawable", ++i);
			}

			//载入动画drawable
			i = -1;
			nodeDrawable = nodeModels.getChildNode("anidrawable", ++i);
			while (!nodeDrawable.isEmpty())
			{
				const char *szDrawableName = nodeDrawable.getAttribute("name");
				AnimationDrawable *drawable = NULL;

				const char *szTarget = nodeDrawable.getAttribute("ref");
				if (szTarget != NULL)
				{
					map<std::string, Drawable *>::iterator it = m_mapImage.find(szTarget);
					if (it != m_mapImage.end())
					{
						AnimationDrawable *targetDrawable = (AnimationDrawable *)(it->second);
						drawable = (AnimationDrawable *)targetDrawable->Clone();
					}
				}
				if (drawable == NULL)
				{
					drawable = new AnimationDrawable();
				}

				const char *szOffset = nodeDrawable.getAttribute("offset");
				long offset = atol(szOffset);
				drawable->setOffset(offset);

				const char *szLoopMode = nodeDrawable.getAttribute("loopMode");
				if (szLoopMode != NULL && strcmp(szLoopMode, "oneShot") == 0)
				{
					drawable->setLoopMode(ELM_ONESHOT);
				}
				else
				{
					drawable->setLoopMode(ELM_REPEAT);
				}

				const char *szGenMipmap = nodeDrawable.getAttribute("genMipmaps");
				bool bGenMipmap = false;
				if (szGenMipmap != NULL && (strcmp(szGenMipmap, "yes") || strcmp(szGenMipmap, "YES")))
				{
					bGenMipmap = true;
				}
				//解析动画信息
				int j = -1;
				XMLNode nodeItem = nodeDrawable.getChildNode("item", ++j);
				while (!nodeItem.isEmpty())
				{
					const char *szImagePath = nodeItem.getAttribute("image");
					sprintf(szFullFile, "%s/%s", szFile, szImagePath);

					std::shared_ptr< CC3DTextureRHI> TexRHI = GetDynamicRHI()->FetchTexture(szFullFile, bGenMipmap);
					if (TexRHI == nullptr)
					{
						TexRHI = GetDynamicRHI()->CreateTextureFromZip(hZip, szImagePath, bGenMipmap);
						GetDynamicRHI()->RecoredTexture(szFullFile, TexRHI);
					}

					const char *szDuring = nodeItem.getAttribute("duration");
					long during = atol(szDuring);

					drawable->appandTex(during, TexRHI);

					nodeItem = nodeDrawable.getChildNode("item", ++j);
				}

				m_mapImage.insert(std::make_pair(szDrawableName, drawable));

				nodeDrawable = nodeModels.getChildNode("anidrawable", ++i);
			}
			//载入背景
			i = -1;
			XMLNode nodeBKG = nodeModels.getChildNode("background", ++i);
			while (!nodeBKG.isEmpty())
			{
				const char *szModelPath = nodeBKG.getAttribute("drawable");
				int x = 0;
				int y = 0;
				int width = 0;
				int height = 0;
				int alignType = EAPT_CC;
				Effect2DRect rectEffect;
				const char *szX = nodeBKG.getAttribute("x");
				const char *szY = nodeBKG.getAttribute("y");
				const char *szWidth = nodeBKG.getAttribute("width");
				const char *szHeight = nodeBKG.getAttribute("height");
				const char *szAlignType = nodeBKG.getAttribute("alignType");

				if (szX != NULL) { x = atoi(szX); }
				if (szY != NULL) { y = atoi(szY); }
				if (szWidth != NULL) { width = atoi(szWidth); }
				if (szHeight != NULL) { height = atoi(szHeight); }
				if (szAlignType != NULL)
				{
					std::string szArrAlignType[] = { "EAPT_LT", "EAPT_LB", "EAPT_RT", "EAPT_RB", "EAPT_CT", "EAPT_CB", "EAPT_LC", "EAPT_RC", "EAPT_CC" };
					for (int index = 0; index<EAPT_MAX; ++index)
					{
						if (strcmp(szAlignType, szArrAlignType[index].c_str()) == 0)
						{
							alignType = index;
							break;
						}
					}
				}

				rectEffect.setRect(x, y, width, height, alignType, false);

				const char *szDrawable = nodeBKG.getAttribute("drawable");
				if (szDrawable != NULL && strlen(szDrawable) > 0)
				{
					map<std::string, Drawable *>::iterator it = m_mapImage.find(szDrawable);
					if (it != m_mapImage.end())
					{
						rectEffect.m_drawable = it->second->Clone();
					}
					else
					{
						rectEffect.m_drawable = NULL;
					}
				}

				m_vBackGround.push_back(rectEffect);

				nodeBKG = nodeModels.getChildNode("background", ++i);
			}


			//载入普通物体
			i = -1;
			XMLNode nodeCocosModel = nodeModels.getChildNode("fsObject", ++i);
			while (!nodeCocosModel.isEmpty())
			{
				const char *szModelPath = nodeCocosModel.getAttribute("file");
				FSObject *pObject = FSObject::createFromZip(szModelPath, hZip, szFile);
				if (pObject != NULL)
				{
					pObject->retain();
					loadFSObject(hZip, szFile, pObject, nodeCocosModel);
					m_vRootObjects.push_back(pObject);
				}

				nodeCocosModel = nodeModels.getChildNode("fsObject", ++i);
			}

			//载入主角
			i = -1;
			XMLNode nodeFacialObject = nodeModels.getChildNode("facialObject", ++i);
			while (!nodeFacialObject.isEmpty())
			{
				const char *szModelPath = nodeFacialObject.getAttribute("file");
				const char *szNeckBone = nodeFacialObject.getAttribute("neckBone");
				const char *szNeckBone2 = nodeFacialObject.getAttribute("neckBone2");
				FacialObject *pFacialObject = FacialObject::createFromZip(szModelPath, hZip, szFile);
				if (pFacialObject != NULL)
				{
					pFacialObject->retain();
					loadFSObject(hZip, szFile, pFacialObject, nodeFacialObject);
					{
						XMLNode animationNode = nodeFacialObject.getChildNode("animation", 0);
						const char* file = animationNode.getAttribute("file");
						if(file) m_animate_file = std::string(file);
						const char *id = animationNode.getAttribute("id");
						if (id) m_id = std::string(id);
						else m_id = "Take 001";
					}
					m_vRootObjects.push_back(pFacialObject);

					if (szNeckBone != NULL)
					{
						pFacialObject->setNeckBone(szNeckBone, szNeckBone2);
					}

					m_pFacialObj = pFacialObject;
				}

				nodeFacialObject = nodeModels.getChildNode("facialObject", ++i);
			}

			//载入灯光
			i = -1;
			XMLNode nodeLight = nodeModels.getChildNode("light", ++i);
			while (!nodeLight.isEmpty())
			{
				BaseLight light;
				memset(&light, 0, sizeof(BaseLight));
				const char *szAmbient = nodeLight.getAttribute("ambient");
				const char *szDiffuse = nodeLight.getAttribute("diffuse");
				const char *szSpecular = nodeLight.getAttribute("specular");
				const char *szOrientation = nodeLight.getAttribute("orientation");
				if (szAmbient != NULL)
				{
					sscanf(szAmbient, "%f,%f,%f,%f", &light.m_vAmbient.x, &light.m_vAmbient.y, &light.m_vAmbient.z, &light.m_vAmbient.w);
				}
				if (szDiffuse != NULL)
				{
					sscanf(szDiffuse, "%f,%f,%f,%f", &light.m_vDiffuse.x, &light.m_vDiffuse.y, &light.m_vDiffuse.z, &light.m_vDiffuse.w);
				}
				if (szSpecular != NULL)
				{
					sscanf(szSpecular, "%f,%f,%f,%f", &light.m_vSpecular.x, &light.m_vSpecular.y, &light.m_vSpecular.z, &light.m_vSpecular.w);
				}
				if (szOrientation != NULL)
				{
					sscanf(szOrientation, "%f,%f,%f", &light.m_vOrientation.x, &light.m_vOrientation.y, &light.m_vOrientation.z);
					light.m_vOrientation.normalize();
				}
				m_vLights.push_back(light);

				nodeLight = nodeModels.getChildNode("light", ++i);
			}

			//载入NPR信息
			m_NPRInfo.initDefault();
			i = -1;
			XMLNode nodeNPR = nodeModels.getChildNode("npr",++i);
			if (!nodeNPR.isEmpty())
			{
				m_NPRInfo.m_bUseNPR = true;
				const char *szLineWidth = nodeNPR.getAttribute("lineWidth");
				const char *szlineColor = nodeNPR.getAttribute("lineColor");
				const char *szZOffset = nodeNPR.getAttribute("zOffset");
				if (szLineWidth != NULL)
				{
					sscanf(szLineWidth, "%f", &m_NPRInfo.m_fLineWidth);
				}
				if (szlineColor != NULL)
				{
					sscanf(szlineColor, "%f,%f,%f,%f", &m_NPRInfo.m_vLineColor.x, &m_NPRInfo.m_vLineColor.y, &m_NPRInfo.m_vLineColor.z, &m_NPRInfo.m_vLineColor.w);
				}
				if (szlineColor != NULL)
				{
					sscanf(szZOffset, "%f,%f,%f", &m_NPRInfo.m_vZOffest.x, &m_NPRInfo.m_vZOffest.y, &m_NPRInfo.m_vZOffest.z);
				}
			}

			//载入相机内参信息
			//m_CameraParam = vec3(atan(0.5) * 2, 1.0, 2000.0);
			m_CameraParam = vec3(0.2094, 1.0, 2000.0);
			i = -1;
			XMLNode nodecamera = nodeModels.getChildNode("camera", ++i);
			if (!nodecamera.isEmpty())
			{
				const char *szfov = nodecamera.getAttribute("fov");
				const char *sznear = nodecamera.getAttribute("near");
				const char *szfar = nodecamera.getAttribute("far");
				if (szfov != NULL)
				{
					sscanf(szfov, "%f", &m_CameraParam.x);
					m_CameraParam.x = m_CameraParam.x*3.14159 / 180.0;
				}
				if (sznear != NULL)
				{
					sscanf(sznear, "%f", &m_CameraParam.y);
				}
				if (szfar != NULL)
				{
					sscanf(szfar, "%f", &m_CameraParam.z);
				}
			}

			DrawableMap_::iterator it = m_mapImage.begin();
			{
				while (it != m_mapImage.end())
				{
					if (it->second != NULL)
					{
						delete it->second;
					}
					++it;
				}
			}
			m_mapImage.clear();
		}

		CloseZip(hZip);
	}

	return true;
}

void SceneInfo::destory()
{
	m_vLights.clear();

	m_pFacialObj = NULL;

	for (int i = 0; i < m_vBackGround.size(); ++i)
	{
		m_vBackGround[i].destory();
	}
	m_vBackGround.clear();

	vector<FSObject *>::iterator it = m_vRootObjects.begin();
	while (it != m_vRootObjects.end())
	{
		(*it)->release();
		++it;
	}
	m_vRootObjects.clear();

	//释放autopool对象
	cocos2d::PoolManager::getInstance()->getCurrentPool()->clear();
}

bool SceneInfo::needRender()
{
	return m_vRootObjects.size() > 0;
}

void SceneInfo::setBackGround(const char *szZipFile, const char *szImg)
{
	HZIP hZip = OpenZip(szZipFile, NULL);
	if (hZip != NULL && m_vBackGround.size() > 0)
	{
		char szFullFile[256];
		sprintf(szFullFile, "%s/%s", szZipFile, szImg);

		bool bGenMipmap = false;

		//防止重复加载图片

		std::shared_ptr< CC3DTextureRHI> TexRHI = GetDynamicRHI()->FetchTexture(szFullFile, bGenMipmap);
		if (TexRHI == nullptr)
		{
			TexRHI = GetDynamicRHI()->CreateTextureFromZip(hZip, szImg, bGenMipmap);
			GetDynamicRHI()->RecoredTexture(szFullFile, TexRHI);
		}

		BitmapDrawable *drawable = new BitmapDrawable(TexRHI);

		if (m_vBackGround.at(0).m_drawable != NULL)
		{
			delete m_vBackGround.at(0).m_drawable;
		}
		m_vBackGround.at(0).m_drawable = drawable;

		CloseZip(hZip);
	}
}

cocos2d::Action* SceneInfo::addAnimation(const string id, const string mode, FacialObject* obj) {
	if(obj->getAnimationAction(id) != nullptr)
		return nullptr;

	HZIP hZip = OpenZip(m_zip_file.c_str(), NULL);
	if (hZip != NULL) {
		//load animation
		cocos2d::Animation3D *animation = cocos2d::Animation3D::createFromZip(m_animate_file, hZip, m_zip_file, m_id);		
		cocos2d::Animate3D *meshAnimate = cocos2d::Animate3D::createWithFrames(animation, 0, 10, 24);
		if (mode == "repeat") {
			cocos2d::RepeatForever *repeatAnimation = cocos2d::RepeatForever::create(meshAnimate);
			if (repeatAnimation)
			{
				obj->addAnimation(id, repeatAnimation);
				CloseZip(hZip);
				return repeatAnimation;
			}
		} else {
			obj->addAnimation(id, meshAnimate);
			CloseZip(hZip);
			return meshAnimate;
		}
	}
	CloseZip(hZip);
	return nullptr;
}

FacialAnimation::FacialAnimation()
{
	m_nLastTime = -1;

	m_bNeedUpdateCameraDraw = true;
	m_arrCameraView = vec4(0.01, 1.0-0.3-0.01, 0.3, 0.3);

	m_pFBO = NULL;
	m_rectDraw = NULL;

	m_pTargetTexture = NULL;

	m_lastFaceRes.numFace = 0;
	m_sceneInfo.m_CameraParam = vec3(atan(0.5) * 2, 1.0, 2000.0);

	m_Reconstruct3D = NULL;

	m_resourcePath = "./";

}

FacialAnimation::~FacialAnimation()
{
	destory();
	SAFEDEL(m_rectDraw);
	SAFEDEL(m_pFBO);
	SAFEDEL(m_pTargetTexture);
	SAFEDEL(m_Reconstruct3D)
}

bool FacialAnimation::loadScene(const char *szFile)
{
	return m_sceneInfo.loadScene(szFile);
}

void FacialAnimation::destory()
{
	m_sceneInfo.destory();
}

void FacialAnimation::setBackGround(const char *szZipFile, const char *szImg)
{
	m_sceneInfo.setBackGround(szZipFile, szImg);
}

void FacialAnimation::SetCameraArea(float x, float y, float w, float h)
{
	m_arrCameraView = vec4(x, y, w, h);
	m_bNeedUpdateCameraDraw = true;
}

void FacialAnimation::SetResourcePath(const char *path)
{
	m_resourcePath = path;
}

void FacialAnimation::updateAvantarParam(float *arr)
{
	if (m_sceneInfo.m_pFacialObj != NULL)
	{
		m_sceneInfo.m_pFacialObj->updateFacialInfo(arr);
	}
}

void FacialAnimation::play(const char *szAnimation, ActionUserCallback pCallBack, void *pUserInfo)
{
	if (m_sceneInfo.m_pFacialObj != NULL)
	{
		m_sceneInfo.m_pFacialObj->playExt(szAnimation, pCallBack, pUserInfo);
	}
}

bool FacialAnimation::renderEffectToTexture(ID3D11ShaderResourceView *pInputTexture, ID3D11Texture2D *pTargetTexture, int width, int height, const ccFDFaceRes *faceRes)
{
	//释放autopool对象
	cocos2d::PoolManager::getInstance()->getCurrentPool()->clear();

	assert(width > 0);
	assert(width > 0);
	prepare();
	if (m_pFBO == NULL || m_pFBO->width() != width || m_pFBO->height() != height || m_pTargetTexture == NULL)
	{
		if (m_pFBO != NULL)
		{
			delete m_pFBO;
		}

		SAFEDESTORY(m_pTargetTexture);

		m_pTargetTexture = new DX11Texture();
		m_pTargetTexture->initTexture(DXGI_FORMAT_R8G8B8A8_UNORM, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET, width, height, NULL, 0, false, true);

		m_pFBO = new DX11FBO();
		m_pFBO->initWithTexture(width, height, true, m_pTargetTexture->getTex());
	}
	if (m_bNeedUpdateCameraDraw)
	{
		m_bNeedUpdateCameraDraw = false;
		if (m_rectDraw == NULL)
		{
			m_rectDraw = new RectDraw();
			m_rectDraw->init(m_arrCameraView.x, m_arrCameraView.y, m_arrCameraView.z, m_arrCameraView.w);
		}
		else
		{
			m_rectDraw->reRect(m_arrCameraView.x, m_arrCameraView.y, m_arrCameraView.z, m_arrCameraView.w);
		}
	}

	if (!m_sceneInfo.needRender())
	{
		return false;
	}

	//float blendFactor[] = { 0.f,0.f,0.f,0.f };
	//DeviceContextPtr->OMSetBlendState(m_pBSEnable, blendFactor, 0xffffffff);




	m_pFBO->bind();
	float ClearColor[4] = {0.0f, 0.0f, 0.0f, 0.0f }; // rgba  
	DeviceContextPtr->ClearRenderTargetView(m_pFBO->getRenderTargetView(), ClearColor);


	//渲染贴纸效果
	renderEffect(width, height, faceRes);



	if (m_arrCameraView.z > 0.0f && m_arrCameraView.w > 0.0f)
	{
		//渲染背景
		m_rectDraw->setShaderTextureView(pInputTexture);
		m_rectDraw->render();
	}

	unsigned int sub = D3D11CalcSubresource(0, 0, 1);
	DeviceContextPtr->ResolveSubresource(
		pTargetTexture,
		sub,
		m_pTargetTexture->getTex(),
		sub,
		DXGI_FORMAT_R8G8B8A8_UNORM
	);

	return true;
}



void FacialAnimation::renderEffect(int width, int height, const ccFDFaceRes *faceRes, int *arrRenderView)
{

	auto faceResTmp = faceRes;
	if (faceRes->numFace == 0)
	{
	//	m_FirstReconstruct = true;
		faceResTmp = &m_lastFaceRes;
	}
	else
	{
		m_lastFaceRes = *faceRes;
	}

	if (m_nLastTime < 0)
	{
		m_nLastTime = ccCurrentTime();
		m_nStartTime = m_nLastTime;
	}
	long during = ccCurrentTime() - m_nStartTime;
	long frameTime = ccCurrentTime() - m_nLastTime;
	m_nLastTime = ccCurrentTime();
	frameTime = max(frameTime, long(0));
	frameTime = min(long(100), frameTime);

	prepare();

	if (arrRenderView != NULL)
	{
		D3D11_VIEWPORT vp;
		vp.Width = (FLOAT)arrRenderView[2];
		vp.Height = (FLOAT)arrRenderView[3];
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = arrRenderView[0];
		vp.TopLeftY = arrRenderView[1];
		DeviceContextPtr->RSSetViewports(1, &vp);
	}

	//绘制2D贴图效果
	for (int i = 0; i<m_sceneInfo.m_vBackGround.size(); ++i)
	{
		m_sceneInfo.m_vBackGround[i].render(width, height, during);
	}


// 	{
// 		ofstream out("./logpos.txt", std::ios::app);
// 		out << faceResTmp->numFace << std::endl;
// 		out << faceRes->numFace << std::endl;
// 
// 		out.close();
// 	}

	if (faceResTmp != NULL && faceResTmp->numFace > 0)
	{
		if (faceRes!=NULL&&faceRes->numFace>0)
		{
			auto lmk = faceRes->arrFace->arrShapeDense;
			auto faceRect = faceRes->arrFace->faceRect;

			m_Reconstruct3D->Run((float*)lmk, width, height, faceRect.right - faceRect.left, faceRect.bottom - faceRect.top, m_FirstReconstruct, 2, 2);
			m_Reconstruct3D->OrthoToPerspectMVP(20.0, 3);
// 			{
// 				ofstream out("./logpos.txt", std::ios::app);
// 				out << m_FirstReconstruct << std::endl;
// 				auto cam_params = m_Reconstruct3D->m_pCameraParamPerspect[MLVR_Smooth3DFace];
// 				out << cam_params [0]<<","<< cam_params[1] <<","<< cam_params[2] << std::endl;
// 				out << std::endl;
// 				out << std::endl;
// 				out.close();
// 			}


			m_FirstReconstruct = false;

		}
		updateAvantarParam(m_Reconstruct3D->m_pFitCoeffsExpress47);
		auto cam_params = m_Reconstruct3D->m_pCameraParamPerspect[MLVR_Smooth3DFace];

		cocos2d::Vector<cocos2d::MeshRenderCmd *> m_renderCMD;

		//更新cocos动画信息
		{
			vector<FSObject *>::iterator it = m_sceneInfo.m_vRootObjects.begin();
			while (it != m_sceneInfo.m_vRootObjects.end())
			{
				FSObject *pObject = (*it);
				pObject->update(frameTime*0.001);
				if (pObject != NULL)
				{
					pObject->visit(m_renderCMD, m_sceneInfo.m_vLights.size()>0,m_sceneInfo.m_NPRInfo.m_bUseNPR);

					if (pObject == m_sceneInfo.m_pFacialObj)
					{
						//float rotate[] = { faceResTmp->arrFace[0].pitch, faceResTmp->arrFace[0].yaw+5, faceResTmp->arrFace[0].roll };

						//printf("%f,%f,%f\n", rotate[0], rotate[1] , rotate[2] );
						float rotate[] = { -cam_params[0] + 5, -cam_params[1], cam_params[2] };
						m_sceneInfo.m_pFacialObj->updateNeckRotate(rotate);

						auto lmk = faceRes->arrFace->arrShapeDense;
						m_sceneInfo.m_pFacialObj->updateEyeRotate((Vector2*)lmk, m_Reconstruct3D->m_pFitCoeffsExpress47+1);
					}
				}

				++it;
			}
			std::sort(m_renderCMD.begin(), m_renderCMD.end(), cocos2d::MeshRenderCmdCmp());
		}

		DeviceContextPtr->ClearDepthStencilView(m_pFBO->getDepthStencilView(), D3D11_CLEAR_DEPTH, 1.0f, 0);

		//构建人脸变换矩阵信息
		float fAngle = atan(0.5) * 2;

		float rate = fAngle / m_sceneInfo.m_CameraParam.x;

		if (height > width)
		{
			fAngle = atan(0.5*height / width) * 2;
		}
		float RealAngle = fAngle /rate;
		XMMATRIX matProjDX = XMMatrixPerspectiveFovRH(RealAngle, width*1.0f / height, m_sceneInfo.m_CameraParam.y, m_sceneInfo.m_CameraParam.z);
		matProjDX = XMMatrixTranspose(matProjDX);
		int faceIndex = 0;
		const ccFDShape68 &faceInfo = faceRes->arrFace[faceIndex];
		Mat4 matRotateX;
		Mat4 matRotateY;
		Mat4 matRotateZ;
		Mat4 matTranslate;
		Mat4 matRotateXYZ;
		Mat4::createRotationX(0 / 180.0f*PI, &matRotateX);
		Mat4::createRotationY(0 / 180.0f*PI, &matRotateY);
		Mat4::createRotationZ(0 / 180.0f*PI, &matRotateZ);

		Mat4::multiply(matRotateX, matRotateY, &matRotateXYZ);
		Mat4::multiply(matRotateXYZ, matRotateZ, &matRotateXYZ);
		Mat4::createTranslation(Vec3(faceRes->arrFace[faceIndex].x / 10, faceRes->arrFace[faceIndex].y / 10 - 160, -392 / 5 * tan(0.5*fAngle)/tan(0.5*RealAngle)), &matTranslate);
		Mat4::multiply(matTranslate, matRotateXYZ, &matRotateXYZ);

		Mat4 matProj;
		memcpy(&matProj, &matProjDX, sizeof(XMMATRIX));

		//Mat4::multiply(matProj, matRotateXYZ, &matRotateXYZ);
		for (size_t i = 0; i < m_renderCMD.size(); ++i)
		{
			
			Mat4 matWorld = matRotateXYZ;
			const Mat4 &matInit = m_sceneInfo.m_pFacialObj->getWTransform();
			Mat4::multiply(matWorld, matInit, &matWorld);
			m_renderCMD.at(i)->updateTransform(matWorld);
		}


		Mat4 matView = Mat4::IDENTITY;
		std::vector<BaseLight> arrLights;

		ContextInst->setCullMode(D3D11_CULL_BACK);
		//绘制cocos模型透明物体
		{
			for (size_t i = 0; i < m_renderCMD.size(); ++i)
			{
				if (!m_renderCMD.at(i)->isTransparent())
					m_renderCMD.at(i)->render(matView, matProj, m_sceneInfo.m_vLights,&m_sceneInfo.m_NPRInfo);
			}
		}

		ContextInst->setCullMode(D3D11_CULL_NONE);
		//绘制cocos模型透明物体
		{
			for (size_t i = 0; i < m_renderCMD.size(); ++i)
			{
				if (m_renderCMD.at(i)->isTransparent())
					m_renderCMD.at(i)->render(matView, matProj, m_sceneInfo.m_vLights, &m_sceneInfo.m_NPRInfo);
			}
		}
	}

	else
	{
		m_FirstReconstruct = true;
	}

	if (arrRenderView != NULL)
	{
		D3D11_VIEWPORT vp;
		vp.Width = (FLOAT)arrRenderView[4];
		vp.Height = (FLOAT)arrRenderView[5];
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		DeviceContextPtr->RSSetViewports(1, &vp);
	}
}

extern const char *s_szStickerShader;
void FacialAnimation::resetAnidrawable(const std::vector<std::string>& file_list) {
	//for_each(file_list.begin(), file_list.end(), [](const string& file) {std::cout << file << std::endl; });
	
	AnimationDrawable* drawable = new AnimationDrawable();
	drawable->setOffset(0);
	drawable->setLoopMode(ELM_REPEAT);

	for(auto file : file_list) {

		std::shared_ptr<CC3DTextureRHI> TexRHI = GetDynamicRHI()->CreateTextureFromFileCPUAcess(file);
		drawable->appandTex(84, TexRHI);
	}
	
	if (m_sceneInfo.m_vBackGround[0].m_drawable) delete m_sceneInfo.m_vBackGround[0].m_drawable;

	m_sceneInfo.m_vBackGround[0].m_drawable = drawable;
}
bool FacialAnimation::prepare()
{
	if (m_Reconstruct3D == NULL)
	{
		m_Reconstruct3D = new mt3dface::MultiLinearVideoReconstructor();

		string path = m_resourcePath + "/3DModels/";
		//string path =  "../BuiltInResource/3DModels/";
		m_Reconstruct3D->LoadModel(path.c_str());
	}
	return true;
}