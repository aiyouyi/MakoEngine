#include "StickerEffect.h"
//#include<vld.h>
#include "Toolbox/DXUtils/DX11Context.h"
#include "FaceDetectorInterface.h"
#include "common.h"
#include "Toolbox/HeaderModelForCull.h"
#include "Toolbox/Helper.h"

#include <assert.h>
#include <map>
#include <algorithm>
#include <iostream>

#include "Toolbox/string_util.h"

using namespace std;


const char *s_szStickerShader = "\
cbuffer ConstantBuffer : register(b0)\
{\
	matrix matWVP;\
	float4 vColor;\
}\
Texture2D txDiffuse : register(t0);\
SamplerState samLinear : register(s0);\
\
struct VS_INPUT\
{\
	float4 Pos : POSITION;\
	float2 Tex : TEXCOORD0;\
};\
struct VS_OUTPUT\
{\
	float4 Pos : SV_POSITION;\
	float4 Color: COLOR; \
	float2 Tex : TEXCOORD0;\
};\
\
VS_OUTPUT VS(VS_INPUT input)\
{\
	VS_OUTPUT output = (VS_OUTPUT)0;\
	output.Pos = mul(matWVP, input.Pos);\
	output.Tex = input.Tex;\
	output.Color = vColor;\
	return output;\
}\
float4 PS(VS_OUTPUT input) : SV_Target\
{\
	return txDiffuse.Sample(samLinear, input.Tex)*input.Color;\
}";

const char *s_szHeaderForCullShader = "\
cbuffer ConstantBuffer : register(b0)\
{\
	matrix matWVP;\
}\
struct VS_INPUT\
{\
	float4 Pos : POSITION;\
};\
struct VS_OUTPUT\
{\
	float4 Pos : SV_POSITION;\
};\
\
VS_OUTPUT VS(VS_INPUT input)\
{\
	VS_OUTPUT output = (VS_OUTPUT)0;\
	output.Pos = mul(matWVP, input.Pos);\
	return output;\
}\
float4 PS(VS_OUTPUT input) : SV_Target\
{\
	return float4(1.0f,1.0f,1.0f,1.0f);\
}";

extern bool loadFSObject(HZIP hZip, const char *szFile, FSObject *pObject, XMLNode &nodeCocosModel);
bool Cocos3DScene::loadFromXML(const XMLNode &nodeModels, HZIP hZip, const char *szFile)
{
	//载入普通物体
	int i = -1;
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

	return true;
}

void Cocos3DScene::destory()
{
	vector<FSObject *>::iterator it = m_vRootObjects.begin();
	while (it != m_vRootObjects.end())
	{
		(*it)->release();
		++it;
	}
	m_vRootObjects.clear();

	//释放autopool对象
	cocos2d::PoolManager::getInstance()->getCurrentPool()->clear();

	m_pFacialObj = NULL;

	//灯光
	m_vLights.clear();
}

void Cocos3DScene::reset()
{
	m_vRootObjects.clear();
	m_pFacialObj = NULL;
	m_vLights.clear();
}


void SEffectInfo::destory()
{
	for (int i = 0; i < m_vEffectModel.size(); ++i)
	{
		m_vEffectModel[i].destory();
	}
	m_vEffectModel.clear();


	for (int i = 0; i < m_v2DEffectModel.size(); ++i)
	{
		m_v2DEffectModel[i].destory();
	}
	m_v2DEffectModel.clear();

	if (m_v2DBGAnimation != NULL)
	{
		delete m_v2DBGAnimation;
		m_v2DBGAnimation = NULL;
	}

	m_cocosScene.destory();
}
bool SEffectInfo::hasObject()
{
	if (m_v2DBGAnimation != NULL || m_v2DEffectModel.size() > 0 || m_vEffectModel.size() > 0 || m_cocosScene.needRender())
	{
		return true;
	}
	return false;
}

void SEffectInfo::reset()
{
	m_v2DBGAnimation = NULL;
	m_v2DEffectModel.clear();
	m_vEffectModel.clear();
	m_cocosScene.reset();
}

class DX11ShaderForHeaderCull : public DX11Shader
{
public:
	DX11ShaderForHeaderCull() {}
	~DX11ShaderForHeaderCull() {}

	virtual bool initInputLayout(ID3DBlob *pVSBlob)
	{
		HRESULT hr = S_OK;
		D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		UINT numElements = ARRAYSIZE(layout);
		hr = DevicePtr->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &m_pVertexLayout);

		return (hr == S_OK);
	}
};


vector<HeaderModel> StickerEffect::m_vHeaderModel;
StickerEffect::StickerEffect()
{
	m_nEffectDuring = -1;
	m_nAniLoopTime = -1;
	m_lpParam = NULL;
	m_pEffectCallback = NULL;
	m_effectLoopMode = ELM_REPEAT;

	m_nStartTime = -1;
	m_nLoopStartTime = -1;

	m_pFBO = NULL;
	m_rectDraw = NULL;

	m_pTargetTexture = NULL;

	m_effectStatus = SES_IDLE;
	m_hLoadThread = 0;

	m_bPrepare = false;
	m_pSamplerLinear = NULL;

	//渲染模型信息
	m_pShader = NULL;
	m_pConstantBuffer = NULL;  //转换矩阵

	m_effectInfo.reset();

	m_pShaderForHeaderCull = NULL;
	m_pConstantBufferForHeaderCull = NULL;  //转换矩阵

	//混合以及buffer操作
	m_pBSEnable = NULL;
	m_pBSWriteDisable = NULL;
	m_pBSDisable = NULL;
	m_pBSDisableWriteDisable = NULL;

	//深度操作
	m_pDepthStateEnable = NULL;
	m_pDepthStateDisable = NULL;
	m_pDepthStateEnableWriteDisable = NULL;

	//效果资源cache大小
	m_nMaxCacheSize = 0;
}


StickerEffect::~StickerEffect()
{
	destory();
}

void StickerEffect::setMaxCacheSize(int nSize)
{
	if (nSize < 0) { nSize = 0; }

	m_nMaxCacheSize = nSize;

	//清理掉不需要的缓存数据
	while(m_listEffectCache.size() > m_nMaxCacheSize)
	{
		SEffectCache *effectCache = m_listEffectCache.front();
		if (effectCache != NULL)
		{
			effectCache->m_effectInfo.destory();
		}

		m_listEffectCache.pop_front();
	}
}

bool StickerEffect::loadEffect(const string &szPath, const string &file, EffectCallback callback, void *lpParam)
{
	if (m_effectStatus == SES_LOADING)
	{
		return false;
	}

	bool bNeedReload = true;
	if (m_szCurrentDir == szPath && m_szCurrentXML == file && m_effectInfo.hasObject())
	{
		bNeedReload = false;
	}
	if (bNeedReload)
	{
		RecycleEffect();
	}

	if (szPath.size() > 0 && file.size()>0)
	{
		if (bNeedReload)
		{
			if (loadFromCache(szPath, file))
			{
				resetAnimation();
			}
			else
			{
				loadEffect_impl(szPath.c_str(), file.c_str());
			}
		}

		m_pEffectCallback = callback;
		m_lpParam = lpParam;

		if (m_pEffectCallback != NULL)
		{
			(*m_pEffectCallback)(EE_START, NULL);
		}

		m_effectStatus = SES_RUN;
	}

	m_szCurrentDir = szPath;
	m_szCurrentXML = file;

	resetAnimation();

	return true;
}

bool StickerEffect::loadEffectFromZip(const string &szZipFile, const string &xml, EffectCallback callback, void *lpParam)
{
	if (m_effectStatus == SES_LOADING)
	{
		return false;
	}

	bool bNeedReload = true;
	if (m_szCurrentDir == szZipFile && m_szCurrentXML == xml && m_effectInfo.hasObject())
	{
		bNeedReload = false;
	}

	if (bNeedReload)
	{
		RecycleEffect();
	}

	if (szZipFile.size() > 0)
	{
		m_pEffectCallback = callback;
		m_lpParam = lpParam;

		if (bNeedReload && !loadFromCache(szZipFile, xml))
		{
			if (szZipFile.size() > 0)
			{
				loadEffectFromZip_impl(szZipFile.c_str(), xml.c_str());
				touchThreadSource();
			}
		}
		else
		{
			if (m_effectInfo.hasObject())
			{
				m_effectStatus = SES_RUN;
				if (m_pEffectCallback != NULL)
				{
					(*m_pEffectCallback)(EE_START, m_lpParam);
				}
			}
			else
			{
				m_effectStatus = SES_IDLE;
			}
		}
	}

	m_szCurrentDir = szZipFile;
	m_szCurrentXML = xml;

	resetAnimation();

	return true;
}

bool StickerEffect::loadEffectFromZip_aysn(const string &szZipFile, const string &xml, EffectCallback callback, void *lpParam)
{
	if (m_effectStatus == SES_LOADING)
	{
		return false;
	}

	bool bNeedReload = true;
	if (m_szCurrentDir == szZipFile && m_szCurrentXML == xml && m_effectInfo.hasObject())
	{
		bNeedReload = false;
	}

	if (bNeedReload)
	{
		RecycleEffect();
	}

	if (szZipFile.size() > 0)
	{
		m_pEffectCallback = callback;
		m_lpParam = lpParam;

		if (bNeedReload && !loadFromCache(szZipFile, xml))
		{
			if (szZipFile.size() > 0)
			{
				loadEffectFromZip_asyn_impl(szZipFile.c_str(), xml.c_str());
				touchThreadSource();
			}
		}
		else
		{
			if (m_effectInfo.hasObject())
			{
				m_effectStatus = SES_RUN;
				if (m_pEffectCallback != NULL)
				{
					(*m_pEffectCallback)(EE_START, m_lpParam);
				}
			}
			else
			{
				m_effectStatus = SES_IDLE;
			}
		}
	}

	m_szCurrentDir = szZipFile;
	m_szCurrentXML = xml;

	resetAnimation();

	return true;
}

bool StickerEffect::loadEffect_impl(const char *szPath, const char *szXMLFile)
{
	char szFullFile[256];
	sprintf(szFullFile, "%s/%s", szPath, szXMLFile);

	XMLResults xResults;
	XMLNode nodeModels = XMLNode::parseFile(szFullFile, "models", &xResults);

	const char *szLoopMode = nodeModels.getAttribute("loopMode");
	const char *szEffectDuring = nodeModels.getAttribute("effectDuring");
	en_EffectLoop_Mode effectLoopMode = ELM_REPEAT;
	long effectDuring = -1;
	if (szLoopMode != NULL && strcmp(szLoopMode, "oneShot") == 0)
	{
		effectLoopMode = ELM_ONESHOT;
	}

	if (szEffectDuring != NULL)
	{
		effectDuring = atoi(szEffectDuring);
	}

	m_effectLoopMode = effectLoopMode;
	m_nEffectDuring = effectDuring;

	typedef map<string, Drawable *> DrawableMap_;
	DrawableMap_ m_mapImage;
	int i = -1;
	XMLNode nodeDrawable = nodeModels.getChildNode("drawable", ++i);
	while (!nodeDrawable.isEmpty())
	{
		const char *szDrawableName = nodeDrawable.getAttribute("name");
		const char *szImagePath = nodeDrawable.getAttribute("image");
		const char *szGenMipmap = nodeDrawable.getAttribute("genMipmaps");
		bool bGenMipmap = false;
		if (szGenMipmap != NULL && (strcmp(szGenMipmap, "yes") || strcmp(szGenMipmap, "YES")))
		{
			bGenMipmap = true;
		}
		sprintf(szFullFile, "%s/%s", szPath, szImagePath);

		//防止重复加载图片
		std::shared_ptr< CC3DTextureRHI> TexRHI = GetDynamicRHI()->FetchTexture(szFullFile, bGenMipmap);

		m_mapImage.insert(std::make_pair(szDrawableName, new BitmapDrawable(TexRHI)));


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
			sprintf(szFullFile, "%s/%s", szPath, szImagePath);
			std::shared_ptr< CC3DTextureRHI> TexRHI = GetDynamicRHI()->FetchTexture(szFullFile, bGenMipmap);

			const char *szDuring = nodeItem.getAttribute("duration");
			long during = atol(szDuring);

			drawable->appandTex(during, TexRHI);

			nodeItem = nodeDrawable.getChildNode("item", ++j);
		}

		m_mapImage.insert(std::make_pair(szDrawableName, drawable));

		nodeDrawable = nodeModels.getChildNode("anidrawable", ++i);
	}

	long aniLoopTime = -1;
	i = -1;
	XMLNode nodeModel = nodeModels.getChildNode("model", ++i);
	while (!nodeModel.isEmpty())
	{
		EffectModel model;
		//const char *szModelName = nodeModel.getAttribute("name");
		const char *szFaceCount = nodeModel.getAttribute("faceCount");
		const char *szVertexCount = nodeModel.getAttribute("vertexCount");
		//const char *szVertexDecl = nodeModel.getAttribute("vertexDecl");
		const char *szDrawable = nodeModel.getAttribute("drawable");
		const char *szColor = nodeModel.getAttribute("color");
		const char *szTransparent = nodeModel.getAttribute("transparent");
		if (szColor != NULL)
		{
			vec4 vColor(1, 1, 1, 1);
			sscanf(szColor, "%f,%f,%f,%f", &vColor.x, &vColor.y, &vColor.z, &vColor.w);
			model.m_fMixColor = vColor;
		}

		if (szTransparent != NULL)
		{
			model.m_bTransparent = false;
			if (strcmp(szTransparent, "YES") == 0 || strcmp(szTransparent, "yes") == 0)
			{
				model.m_bTransparent = true;
			}
		}

		if (szDrawable != NULL && strlen(szDrawable) > 0)
		{
			map<string, Drawable *>::iterator it = m_mapImage.find(szDrawable);
			if (it != m_mapImage.end())
			{
				model.m_drawable = it->second->Clone();
				if (model.m_drawable->getDuring() > aniLoopTime)
				{
					aniLoopTime = model.m_drawable->getDuring();
				}
			}
			else
			{
				model.m_drawable = NULL;
			}
		}

		const char *szModelRef = nodeModel.getAttribute("modeRef");
		if (szModelRef != NULL)
		{
			sprintf(szFullFile, "%s/%s", szPath, szModelRef);
			FILE *file = fopen(szFullFile, "rb");
			if (file != NULL)
			{
				unsigned int nCode;
				int version;
				int nFaceCount;
				int nVertexCount;
				fread(&nCode, sizeof(unsigned int), 1, file);
				fread(&version, sizeof(int), 1, file);
				if (nCode == 0x1f1f1f0f && version == 1)
				{
					fread(&nVertexCount, sizeof(int), 1, file);

					model.m_nVertex = nVertexCount;
					model.m_arrVertices = new EffectVertex[nVertexCount];
					for (int indexVertex = 0; indexVertex < nVertexCount; ++indexVertex)
					{
						fread(&(model.m_arrVertices[indexVertex].vPos), sizeof(vec3), 1, file);
						model.m_arrVertices[indexVertex].vPos.z = -model.m_arrVertices[indexVertex].vPos.z;
					}
					for (int indexVertex = 0; indexVertex < nVertexCount; ++indexVertex)
					{
						fread(&(model.m_arrVertices[indexVertex].vUV), sizeof(vec2), 1, file);
						vec2 &vUV = model.m_arrVertices[indexVertex].vUV;
						if (vUV.x<0.0f || vUV.x > 1.0f)
						{
							vUV.x -= floor(vUV.x);
						}
						if (vUV.y<0.0f || vUV.y > 1.0f)
						{
							vUV.y -= floor(vUV.y);
						}
						vUV.y = 1.0f - vUV.y;
					}

					fread(&nFaceCount, sizeof(int), 1, file);
					model.m_nFaces = nFaceCount;
					model.m_arrIndex = new unsigned short[nFaceCount * 3];
					fread(model.m_arrIndex, sizeof(short), nFaceCount * 3, file);
				}

				m_effectInfo.m_vEffectModel.push_back(model);
				fclose(file);
			}
		}
		else
		{
			int nFaceCount = atoi(szFaceCount);
			int nVertexCount = atoi(szVertexCount);
			//int nVertexDecl = atoi(szVertexDecl);

			if (nFaceCount > 0 && nVertexCount > 0)
			{
				model.m_nFaces = nFaceCount;
				model.m_nVertex = nVertexCount;
				model.m_arrVertices = new EffectVertex[nVertexCount];
				model.m_arrIndex = new unsigned short[nFaceCount * 3];

				int j = -1;
				XMLNode nodeVertex = nodeModel.getChildNode("vertex", ++j);
				while (!nodeVertex.isEmpty())
				{
					const char *szPos = nodeVertex.getAttribute("pos");
					const char *szUV = nodeVertex.getAttribute("uv");

					vec2 vUV;
					vec3 vPos;
					sscanf(szPos, "%f,%f,%f", &vPos.x, &vPos.y, &vPos.z);
					sscanf(szUV, "%f,%f", &vUV.x, &vUV.y);
					if (vUV.x<0.0f || vUV.x > 1.0f)
					{
						vUV.x -= floor(vUV.x);
					}
					if (vUV.y<0.0f || vUV.y > 1.0f)
					{
						vUV.y -= floor(vUV.y);
					}

					model.m_arrVertices[j].vPos = vPos;
					model.m_arrVertices[j].vPos.z = -vPos.z;
					model.m_arrVertices[j].vUV = vUV;
					model.m_arrVertices[j].vUV.y = 1.0f - vUV.y;
					nodeVertex = nodeModel.getChildNode("vertex", ++j);
				}

				j = -1;
				XMLNode nodeIndex = nodeModel.getChildNode("face", ++j);
				while (!nodeIndex.isEmpty())
				{
					const char *szIndex = nodeIndex.getAttribute("index");
					int index[3];
					sscanf(szIndex, "%d,%d,%d", index, index + 1, index + 2);

					model.m_arrIndex[j * 3] = (short)index[0];
					model.m_arrIndex[j * 3 + 1] = (short)index[1];
					model.m_arrIndex[j * 3 + 2] = (short)index[2];
					nodeIndex = nodeModel.getChildNode("face", ++j);
				}
				m_effectInfo.m_vEffectModel.push_back(model);
				//NSLog(@"%ld", m_vEffectModel.size());
			}
		}

		nodeModel = nodeModels.getChildNode("model", ++i);
	}

	std::sort(m_effectInfo.m_vEffectModel.begin(), m_effectInfo.m_vEffectModel.end(), EffectModel::ModeComp());

	for (int i = 0; i < m_effectInfo.m_vEffectModel.size(); ++i)
	{
		EffectModel &effectModel = m_effectInfo.m_vEffectModel[i];
		//创建渲染buffer
		D3D11_BUFFER_DESC verBufferDesc;
		memset(&verBufferDesc, 0, sizeof(D3D11_BUFFER_DESC));
		verBufferDesc.ByteWidth = sizeof(EffectVertex) * effectModel.m_nVertex;
		verBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		verBufferDesc.CPUAccessFlags = 0;
		verBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		D3D11_SUBRESOURCE_DATA vertexInitData;
		memset(&vertexInitData, 0, sizeof(D3D11_SUBRESOURCE_DATA));
		vertexInitData.pSysMem = effectModel.m_arrVertices;
		DevicePtr->CreateBuffer(&verBufferDesc, &vertexInitData, &effectModel.m_rectVerticeBuffer);


		//创建索引buffer
		D3D11_BUFFER_DESC indexBufferDesc;
		memset(&indexBufferDesc, 0, sizeof(D3D11_BUFFER_DESC));
		indexBufferDesc.ByteWidth = sizeof(WORD) * effectModel.m_nFaces * 3;
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags = 0;
		indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		D3D11_SUBRESOURCE_DATA indexInitData;
		memset(&indexInitData, 0, sizeof(D3D11_SUBRESOURCE_DATA));
		indexInitData.pSysMem = effectModel.m_arrIndex;
		DevicePtr->CreateBuffer(&indexBufferDesc, &indexInitData, &effectModel.m_rectIndexBuffer);
	}

	//解析加载2d矩形绘图
	i = -1;
	XMLNode nodeRect = nodeModels.getChildNode("rect", ++i);
	while (!nodeRect.isEmpty())
	{
		/*<rect name="cao" drawable="cao" x="100" y="100" width="100" height="100" alignType="EAPT_LT">
		</rect>
		*/
		Effect2DRect rectEffect;
		const char *szX = nodeRect.getAttribute("x");
		const char *szY = nodeRect.getAttribute("y");
		const char *szWidth = nodeRect.getAttribute("width");
		const char *szHeight = nodeRect.getAttribute("height");
		const char *szAlignType = nodeRect.getAttribute("alignType");

		int x = atoi(szX);
		int y = atoi(szY);
		int width = atoi(szWidth);
		int height = atoi(szHeight);
		int alignType = EAPT_CC;
		if (szAlignType != NULL)
		{
			string szArrAlignType[] = { "EAPT_LT", "EAPT_LB", "EAPT_RT", "EAPT_RB", "EAPT_CT", "EAPT_CB", "EAPT_LC", "EAPT_RC", "EAPT_CC" };
			for (int index = 0; index<EAPT_MAX; ++index)
			{
				if (strcmp(szAlignType, szArrAlignType[index].c_str()) == 0)
				{
					alignType = index;
					break;
				}
			}
		}

		rectEffect.setRect(x, y, width, height, alignType);

		const char *szDrawable = nodeRect.getAttribute("drawable");
		if (szDrawable != NULL && strlen(szDrawable) > 0)
		{
			map<std::string, Drawable *>::iterator it = m_mapImage.find(szDrawable);
			if (it != m_mapImage.end())
			{
				rectEffect.m_drawable = it->second->Clone();
				if (rectEffect.m_drawable->getDuring() > aniLoopTime)
				{
					aniLoopTime = rectEffect.m_drawable->getDuring();
				}
			}
			else
			{
				rectEffect.m_drawable = NULL;
			}
		}

		m_effectInfo.m_v2DEffectModel.push_back(rectEffect);
		nodeRect = nodeModels.getChildNode("rect", ++i);
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

	m_nAniLoopTime = aniLoopTime;

	return (m_effectInfo.m_vEffectModel.size() > 0 || m_effectInfo.m_v2DEffectModel.size() > 0);
}


DX11Texture *CreateFromZIP(HZIP hZip, const char *szImagePath, bool bGenMipmap)
{
	DX11Texture *pTex = new DX11Texture();
	pTex->initTextureFromZip(hZip, szImagePath, bGenMipmap);
	return pTex;
}

DWORD WINAPI ThreadFun(LPVOID pM)
{
	SResourceAsyn *pResourceAsyn = (SResourceAsyn *)pM;

	const char *szPath = pResourceAsyn->szPath.c_str();
	const char *szXMLFile = pResourceAsyn->szXMLFile.c_str();
	const char *szDesc = pResourceAsyn->szDesc.c_str();

	std::wstring wPath = core::u8_ucs2(pResourceAsyn->szPath);

	std::ifstream inStream(wPath, ios_base::in | ios_base::binary);
	std::stringstream buffer;
	buffer << inStream.rdbuf();
	std::string contents(buffer.str());

	HZIP hZip = nullptr;
	if (contents.empty())
	{
		hZip = OpenZip(szPath, nullptr);
	}
	else
	{
		hZip = OpenZip((char*)contents.data(), contents.length(), nullptr);
	}

	if (hZip != NULL)
	{
		int index;
		ZIPENTRY ze;
		long effectDuring = -1;
		if (strlen(szDesc) > 0 && ZR_OK == FindZipItem(hZip, szDesc, true, &index, &ze))
		{
			char *pDataBuffer = new char[ze.unc_size];
			ZRESULT res = UnzipItem(hZip, index, pDataBuffer, ze.unc_size);
			if (res == ZR_OK)
			{
				XMLResults xResults;
				XMLNode nodeModels = XMLNode::parseBuffer(pDataBuffer, ze.unc_size, "giftDesc", &xResults);
				const char *effectXML = nodeModels.getAttribute("effectXML");
				const char *szDuring = nodeModels.getAttribute("during");
				const char *effectXML_V2 = nodeModels.getAttribute("effectXML_V2");
				if (effectXML != NULL)
				{
					pResourceAsyn->szXMLFile = effectXML;
					szXMLFile = pResourceAsyn->szXMLFile.c_str();
				}
				if (effectXML_V2 != NULL)
				{
					pResourceAsyn->szXMLFile = effectXML_V2;
					szXMLFile = pResourceAsyn->szXMLFile.c_str();
				}
				if (szDuring != NULL)
				{
					effectDuring = atoi(szDuring);
				}
			}

			delete[]pDataBuffer;
		}

		if (ZR_OK == FindZipItem(hZip, pResourceAsyn->szXMLFile.c_str(), true, &index, &ze))
		{
			char *pDataBuffer = new char[ze.unc_size];
			ZRESULT res = UnzipItem(hZip, index, pDataBuffer, ze.unc_size);
			if (res != ZR_OK)
			{
				delete[]pDataBuffer;
				CloseZip(hZip);

				pResourceAsyn->status = SES_READY;
				return 0;
			}

			//解析xml
			XMLResults xResults;
			XMLNode nodeModels = XMLNode::parseBuffer(pDataBuffer, ze.unc_size, "models", &xResults);
			delete[]pDataBuffer;
			const char *szLoopMode = nodeModels.getAttribute("loopMode");
			const char *szEffectDuring = nodeModels.getAttribute("effectDuring");
			en_EffectLoop_Mode effectLoopMode = ELM_REPEAT;
			if (szLoopMode != NULL && strcmp(szLoopMode, "oneShot")==0)
			{
				effectLoopMode = ELM_ONESHOT;
			}

			if (szEffectDuring != NULL)
			{
				effectDuring = atoi(szEffectDuring);
			}
			pResourceAsyn->effectLoopMode = effectLoopMode;
			pResourceAsyn->m_nEffectDuring = effectDuring;

			char szFullFile[256];
			sprintf(szFullFile, "%s/%s", szPath, szXMLFile);

			typedef map<string, Drawable *> DrawableMap_;
			DrawableMap_ m_mapImage;
			int i = -1;
			XMLNode nodeDrawable = nodeModels.getChildNode("drawable", ++i);
			while (!nodeDrawable.isEmpty())
			{
				const char *szDrawableName = nodeDrawable.getAttribute("name");
				const char *szImagePath = nodeDrawable.getAttribute("image");

				sprintf(szFullFile, "%s/%s", pResourceAsyn->szPath.c_str(), szImagePath);

				const char *szGenMipmap = nodeDrawable.getAttribute("genMipmaps");
				bool bGenMipmap = false;
				if (szGenMipmap != NULL && (strcmp(szGenMipmap, "yes")==0 || strcmp(szGenMipmap, "YES")==0))
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

				m_mapImage.insert(std::make_pair(szDrawableName, new BitmapDrawable(TexRHI)));

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
				if (szGenMipmap != NULL && (strcmp(szGenMipmap, "yes")==0 || strcmp(szGenMipmap, "YES")==0))
				{
					bGenMipmap = true;
				}

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
						for (; iStart <= nCount; iStart += nStep)
						{
							sprintf(szImagePath, szItems, iStart);

							sprintf(szFullFile, "%s/%s", pResourceAsyn->szPath.c_str(), szImagePath);

							std::shared_ptr< CC3DTextureRHI> TexRHI = GetDynamicRHI()->FetchTexture(szFullFile, bGenMipmap);
							if (TexRHI == nullptr)
							{
								TexRHI = GetDynamicRHI()->CreateTextureFromZip(hZip, szImagePath, bGenMipmap);
								GetDynamicRHI()->RecoredTexture(szFullFile, TexRHI);
							}

							long during = nDuring;

							drawable->appandTex(during, TexRHI);

						}
					}
				}

				//解析动画信息
				int j = -1;
				XMLNode nodeItem = nodeDrawable.getChildNode("item", ++j);
				while (!nodeItem.isEmpty())
				{
					const char *szImagePath = nodeItem.getAttribute("image");
					sprintf(szFullFile, "%s/%s", pResourceAsyn->szPath.c_str(), szImagePath);

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

				m_mapImage.insert(make_pair(szDrawableName, drawable));

				nodeDrawable = nodeModels.getChildNode("anidrawable", ++i);
			}


			long aniLoopTime = -1;
			i = -1;
			XMLNode nodeModel = nodeModels.getChildNode("model", ++i);
			while (!nodeModel.isEmpty())
			{
				EffectModel model;
				//const char *szModelName = nodeModel.getAttribute("name");
				const char *szFaceCount = nodeModel.getAttribute("faceCount");
				const char *szVertexCount = nodeModel.getAttribute("vertexCount");
				//const char *szVertexDecl = nodeModel.getAttribute("vertexDecl");
				const char *szDrawable = nodeModel.getAttribute("drawable");
				const char *szColor = nodeModel.getAttribute("color");
				const char *szTransparent = nodeModel.getAttribute("transparent");

				const char *szLayer = nodeModel.getAttribute("Layer");
				if (szLayer != NULL)
				{
					model.m_Layer = atoi(szLayer);
				}

				if (szColor != NULL)
				{
					vec4 vColor(1, 1, 1, 1);
					sscanf(szColor, "%f,%f,%f,%f", &vColor.x, &vColor.y, &vColor.z, &vColor.w);
					model.m_fMixColor = vColor;
				}

				if (szTransparent != NULL)
				{
					model.m_bTransparent = false;
					if (strcmp(szTransparent, "YES") == 0 || strcmp(szTransparent, "yes") == 0)
					{
						model.m_bTransparent = true;
					}
				}

				if (szDrawable != NULL && strlen(szDrawable) > 0)
				{
					map<string, Drawable *>::iterator it = m_mapImage.find(szDrawable);
					if (it != m_mapImage.end())
					{
						model.m_drawable = it->second->Clone();
						if (model.m_drawable->getDuring() > aniLoopTime)
						{
							//aniLoopTime = model.m_drawable->getDuring();
						}
					}
					else
					{
						model.m_drawable = NULL;
					}
				}

				const char *szModelRef = nodeModel.getAttribute("modeRef");
				if (szModelRef != NULL)
				{
					if (ZR_OK == FindZipItem(hZip, szModelRef, true, &index, &ze))
					{
						char *pDataBuffer = new char[ze.unc_size];
						ZRESULT res = UnzipItem(hZip, index, pDataBuffer, ze.unc_size);
						if (res == ZR_OK)
						{
							unsigned int nCode;
							int version;
							int nFaceCount;
							int nVertexCount;

							char *pDataTemp = pDataBuffer;
							memcpy(&nCode, pDataTemp, sizeof(unsigned int)); pDataTemp += 4;
							memcpy(&version, pDataTemp, sizeof(int)); pDataTemp += 4;
							if (nCode == 0x1f1f1f0f && version == 1)
							{
								memcpy(&nVertexCount, pDataTemp, sizeof(int)); pDataTemp += 4;

								model.m_nVertex = nVertexCount;
								model.m_arrVertices = new EffectVertex[nVertexCount];
								float *fDataTemp = (float *)pDataTemp;
								for (int indexVertex = 0; indexVertex < nVertexCount; ++indexVertex)
								{
									model.m_arrVertices[indexVertex].vPos = vec3(fDataTemp[0], fDataTemp[1], fDataTemp[2]);
									fDataTemp += 3;
								}
								for (int indexVertex = 0; indexVertex < nVertexCount; ++indexVertex)
								{
									model.m_arrVertices[indexVertex].vUV = vec2(fDataTemp[0], fDataTemp[1]);
									vec2 &vUV = model.m_arrVertices[indexVertex].vUV;
									if (vUV.x<0.0f || vUV.x > 1.0f)
									{
										vUV.x -= floor(vUV.x);
									}
									if (vUV.y<0.0f || vUV.y > 1.0f)
									{
										vUV.y -= floor(vUV.y);
									}
									vUV.y = 1.0f - vUV.y;

									fDataTemp += 2;
								}
								pDataTemp += sizeof(EffectVertex)*nVertexCount;

								//
								memcpy(&nFaceCount, pDataTemp, sizeof(int)); pDataTemp += 4;
								model.m_nFaces = nFaceCount;
								model.m_arrIndex = new unsigned short[nFaceCount * 3];
								memcpy(model.m_arrIndex, pDataTemp, sizeof(short)*nFaceCount * 3);
							}

							pResourceAsyn->m_effectInfo.m_vEffectModel.push_back(model);
						}
						delete[]pDataBuffer;
					}
				}
				else
				{
					int nFaceCount = atoi(szFaceCount);
					int nVertexCount = atoi(szVertexCount);
					//int nVertexDecl = atoi(szVertexDecl);

					if (nFaceCount > 0 && nVertexCount > 0)
					{
						model.m_nFaces = nFaceCount;
						model.m_nVertex = nVertexCount;
						model.m_arrVertices = new EffectVertex[nVertexCount];
						model.m_arrIndex = new unsigned short[nFaceCount * 3];

						int j = -1;
						XMLNode nodeVertex = nodeModel.getChildNode("vertex", ++j);
						while (!nodeVertex.isEmpty())
						{
							const char *szPos = nodeVertex.getAttribute("pos");
							const char *szUV = nodeVertex.getAttribute("uv");

							vec2 vUV;
							vec3 vPos;
							sscanf(szPos, "%f,%f,%f", &vPos.x, &vPos.y, &vPos.z);
							sscanf(szUV, "%f,%f", &vUV.x, &vUV.y);
							if (vUV.x<0.0f || vUV.x > 1.0f)
							{
								vUV.x -= floor(vUV.x);
							}
							if (vUV.y<0.0f || vUV.y > 1.0f)
							{
								vUV.y -= floor(vUV.y);
							}

							model.m_arrVertices[j].vPos = vPos;
							model.m_arrVertices[j].vPos.z = vPos.z;
							model.m_arrVertices[j].vUV = vUV;
							model.m_arrVertices[j].vUV.y = 1.0f - vUV.y;
							nodeVertex = nodeModel.getChildNode("vertex", ++j);
						}

						j = -1;
						XMLNode nodeIndex = nodeModel.getChildNode("face", ++j);
						while (!nodeIndex.isEmpty())
						{
							const char *szIndex = nodeIndex.getAttribute("index");
							int index[3];
							sscanf(szIndex, "%d,%d,%d", index, index + 1, index + 2);

							model.m_arrIndex[j * 3] = (short)index[0];
							model.m_arrIndex[j * 3 + 1] = (short)index[1];
							model.m_arrIndex[j * 3 + 2] = (short)index[2];
							nodeIndex = nodeModel.getChildNode("face", ++j);
						}
						pResourceAsyn->m_effectInfo.m_vEffectModel.push_back(model);
						//NSLog(@"%ld", m_vEffectModel.size());
					}
				}
				nodeModel = nodeModels.getChildNode("model", ++i);
			}

			std::sort(pResourceAsyn->m_effectInfo.m_vEffectModel.begin(), pResourceAsyn->m_effectInfo.m_vEffectModel.end(), EffectModel::ModeComp());

			for (int i = 0; i < pResourceAsyn->m_effectInfo.m_vEffectModel.size(); ++i)
			{
				EffectModel &effectModel = pResourceAsyn->m_effectInfo.m_vEffectModel[i];
				//创建渲染buffer
				D3D11_BUFFER_DESC verBufferDesc;
				memset(&verBufferDesc, 0, sizeof(D3D11_BUFFER_DESC));
				verBufferDesc.ByteWidth = sizeof(EffectVertex) * effectModel.m_nVertex;
				verBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
				verBufferDesc.CPUAccessFlags = 0;
				verBufferDesc.Usage = D3D11_USAGE_DEFAULT;
				D3D11_SUBRESOURCE_DATA vertexInitData;
				memset(&vertexInitData, 0, sizeof(D3D11_SUBRESOURCE_DATA));
				vertexInitData.pSysMem = effectModel.m_arrVertices;
				DevicePtr->CreateBuffer(&verBufferDesc, &vertexInitData, &effectModel.m_rectVerticeBuffer);


				//创建索引buffer
				D3D11_BUFFER_DESC indexBufferDesc;
				memset(&indexBufferDesc, 0, sizeof(D3D11_BUFFER_DESC));
				indexBufferDesc.ByteWidth = sizeof(WORD) * effectModel.m_nFaces * 3;
				indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
				indexBufferDesc.CPUAccessFlags = 0;
				indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
				D3D11_SUBRESOURCE_DATA indexInitData;
				memset(&indexInitData, 0, sizeof(D3D11_SUBRESOURCE_DATA));
				indexInitData.pSysMem = effectModel.m_arrIndex;
				DevicePtr->CreateBuffer(&indexBufferDesc, &indexInitData, &effectModel.m_rectIndexBuffer);
			}

			//解析背景效果元素
			i = -1;
			XMLNode nodeBGEffect = nodeModels.getChildNode("bgEffect", ++i);
			if(!nodeBGEffect.isEmpty())
			{
				const char *szDrawable = nodeBGEffect.getAttribute("drawable");
				map<string, Drawable *>::iterator it = m_mapImage.find(szDrawable);
				if (it != m_mapImage.end())
				{
					pResourceAsyn->m_effectInfo.m_v2DBGAnimation = it->second->Clone();
				}
				else
				{
					pResourceAsyn->m_effectInfo.m_v2DBGAnimation = NULL;
				}
			}

			//解析加载2d矩形绘图
			i = -1;
			XMLNode nodeRect = nodeModels.getChildNode("rect", ++i);
			while (!nodeRect.isEmpty())
			{
				/*<rect name="cao" drawable="cao" x="100" y="100" width="100" height="100" alignType="EAPT_LT">
				</rect>
				*/
				Effect2DRect rectEffect;
				const char *szX = nodeRect.getAttribute("x");
				const char *szY = nodeRect.getAttribute("y");
				const char *szWidth = nodeRect.getAttribute("width");
				const char *szHeight = nodeRect.getAttribute("height");
				const char *szAlignType = nodeRect.getAttribute("alignType");

				int x = 0;
				int y = 0;
				if (szX != NULL && strncmp(szX, "rand", strlen("rand")) == 0)
				{
					int r1, r2;
					sscanf(szX, "rand(%d,%d)", &r1, &r2);
					x = r1 + rand() % (r2 - r1 + 1);
				//	printf("a");
				}
				else
				{
					x = atoi(szX);
				}

				if (szY != NULL && strncmp(szY, "rand", strlen("rand")) == 0)
				{
					int r1, r2;
					sscanf(szY, "rand(%d,%d)", &r1, &r2);
					y = r1 + rand() % (r2 - r1 + 1);
				//	printf("a");
				}
				else
				{
					y = atoi(szY);
				}
				
				int width = atoi(szWidth);
				int height = atoi(szHeight);
				int alignType = EAPT_CC;
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

				rectEffect.setRect(x, y, width, height, alignType);

				const char *szDrawable = nodeRect.getAttribute("drawable");
				if (szDrawable != NULL && strlen(szDrawable) > 0)
				{
					map<std::string, Drawable *>::iterator it = m_mapImage.find(szDrawable);
					if (it != m_mapImage.end())
					{
						rectEffect.m_drawable = it->second->Clone();
						if (rectEffect.m_drawable->getDuring() > aniLoopTime)
						{
							//aniLoopTime = rectEffect.m_drawable->getDuring();
						}
					}
					else
					{
						rectEffect.m_drawable = NULL;
					}
				}

				const char *szLayer = nodeRect.getAttribute("layer");
				if (szLayer != NULL)
				{
					rectEffect.m_Layer = atoi(szLayer);
				}

				pResourceAsyn->m_effectInfo.m_v2DEffectModel.push_back(rectEffect);
				nodeRect = nodeModels.getChildNode("rect", ++i);
			}

			//解析加载2d矩形绘图
			i = -1;
			nodeRect = nodeModels.getChildNode("scene", ++i);
			if (!nodeRect.isEmpty())
			{
				pResourceAsyn->m_effectInfo.m_cocosScene.loadFromXML(nodeRect, hZip, szPath);
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

			pResourceAsyn->m_nAniLoopTime = aniLoopTime;
		}
	}

	CloseZip(hZip);

	pResourceAsyn->status = SES_READY;
	return 1;
}


void StickerEffect::loadEffectFromZip_impl(const char *szPath, const char *szXMLFile)
{
	if (m_effectStatus == SES_LOADING)
	{
		return;
	}

	//free状态
	m_resourceAsyn.reset();
	m_resourceAsyn.szPath = szPath;
	m_resourceAsyn.szXMLFile = szXMLFile;
	if (strlen(szXMLFile) == 0)
	{
		m_resourceAsyn.szDesc = "giftDesc.xml";
	}

	ThreadFun(&m_resourceAsyn);
}

void StickerEffect::loadEffectFromZip_asyn_impl(const char *szPath, const char *szXMLFile)
{
	if (m_effectStatus == SES_LOADING)
	{
		return;
	}

	if (m_hLoadThread != 0)
	{
		CloseHandle(m_hLoadThread);
		m_hLoadThread = 0;
	}

	//free状态
	m_resourceAsyn.reset();
	m_resourceAsyn.szPath = szPath;
	m_resourceAsyn.szXMLFile = szXMLFile;
	if (strlen(szXMLFile) == 0)
	{
		m_resourceAsyn.szDesc = "giftDesc.xml";
	}

	m_hLoadThread = CreateThread(NULL, 0, ThreadFun, &m_resourceAsyn, 0/*CREATE_SUSPENDED*/, NULL);

	m_effectStatus = SES_LOADING;
}

void StickerEffect::RecycleEffect()
{
	if (m_nMaxCacheSize <= 0)
	{
		destroyEffect();
		return;
	}

	if (!m_effectInfo.hasObject())
	{
		return;
	}

	list<SEffectCache *>::iterator it = m_listEffectCache.begin();
	while(it != m_listEffectCache.end())
	{
		SEffectCache *pCache = (*it);
		if (m_szCurrentDir == pCache->szPath && m_szCurrentXML == pCache->szXMLFile)
		{
			return;
		}
		++it;
	}

	SEffectCache *pCache = new SEffectCache();
	pCache->m_effectInfo = m_effectInfo;
	pCache->szPath = m_szCurrentDir;
	pCache->szXMLFile = m_szCurrentXML;
	pCache->nEffectDuring = m_nEffectDuring;
	pCache->nAniLoopTime = m_nAniLoopTime;
	pCache->effectLoopMode = m_effectLoopMode;


	m_listEffectCache.push_back(pCache);

	m_effectInfo.reset();

	setMaxCacheSize(m_nMaxCacheSize);
}

bool StickerEffect::loadFromCache(const std::string &szZipFile, const std::string &xml)
{
	list<SEffectCache *>::iterator it = m_listEffectCache.begin();
	while (it != m_listEffectCache.end())
	{
		SEffectCache *pCache = (*it);
		if (szZipFile == pCache->szPath && xml == pCache->szXMLFile)
		{
			m_effectInfo = (*it)->m_effectInfo;
			m_nEffectDuring = pCache->nEffectDuring;
			m_nAniLoopTime = pCache->nAniLoopTime;
			m_effectLoopMode = pCache->effectLoopMode;

			m_listEffectCache.erase(it);
			delete pCache;
			return true;
		}

		++it;
	}

	return false;
}

void StickerEffect::touchThreadSource()
{
	if (m_resourceAsyn.status == SES_READY)
	{
		CloseHandle(m_hLoadThread);
		m_hLoadThread = 0;

		m_effectInfo = m_resourceAsyn.m_effectInfo;

		m_nEffectDuring = m_resourceAsyn.m_nEffectDuring;
		m_nAniLoopTime = m_resourceAsyn.m_nAniLoopTime;
		m_effectLoopMode = m_resourceAsyn.effectLoopMode;

		resetAnimation();

		m_resourceAsyn.status = SES_IDLE;
		m_resourceAsyn.m_effectInfo.reset();

		if (m_effectInfo.hasObject())
		{
			m_effectStatus = SES_RUN;
			if (m_pEffectCallback != NULL)
			{
				(*m_pEffectCallback)(EE_START, m_lpParam);
			}
		}
		else
		{
			m_effectStatus = SES_IDLE;
		}
	}
}

void StickerEffect::resetAnimation()
{
	m_nLoopStartTime = m_nStartTime = -1;
}

void StickerEffect::destroyEffect()
{
	m_effectInfo.destory();
}

void StickerEffect::destroyCacheEffect()
{
	int nCacheSize = m_nMaxCacheSize;
	setMaxCacheSize(0);
	setMaxCacheSize(nCacheSize);
}

void StickerEffect::destory()
{
	if (m_hLoadThread != NULL)
	{
		WaitForSingleObject(m_hLoadThread, 5000);
		::CloseHandle(m_hLoadThread);

		touchThreadSource();
	}

	for (int i = 0; i < m_vHeaderModel.size(); ++i)
	{
		m_vHeaderModel[i].destory();
	}
	m_vHeaderModel.clear();

	destroyEffect();

	destroyCacheEffect();

	SAFEDEL(m_pFBO);
	SAFEDEL(m_rectDraw);
	SAFEDEL(m_pShader);

	SAFEDEL(m_pTargetTexture);

	m_bPrepare = true;

	SAFERALEASE(m_pSamplerLinear);
	SAFERALEASE(m_pConstantBuffer);  //转换矩阵

	SAFEDEL(m_pShaderForHeaderCull);
	SAFERALEASE(m_pConstantBufferForHeaderCull);  //转换矩阵

	SAFERALEASE(m_pBSEnable);
	SAFERALEASE(m_pBSWriteDisable);
	SAFERALEASE(m_pBSDisable);
	SAFERALEASE(m_pBSDisableWriteDisable);

	SAFERALEASE(m_pDepthStateEnable);
	SAFERALEASE(m_pDepthStateDisable);
	SAFERALEASE(m_pDepthStateEnableWriteDisable);
}


struct FaceInfoSort
{
	float zDepth;
	int index;
};
inline bool compare_face_depth(const FaceInfoSort& a, const FaceInfoSort& b)
{
	return a.zDepth > b.zDepth;
}

inline bool BTLayer_Func(int layer)
{
	return layer < 0;
}
inline bool CTLayer_Func(int layer)
{
	return layer == 0;
}
inline bool TPLayer_Func(int layer)
{
	return layer > 0;
}
inline bool CTPLayer_Func(int layer)
{
	return layer >= 0;
}

void StickerEffect::renderEffect(int width, int height, const ccFDFaceRes *faceRes, void *pExtInfo, int *arrRenderView)
{
	prepare();
	touchThreadSource();

	if (m_effectStatus != SES_RUN)
	{
		return;
	}

	if (m_nStartTime < 0)
	{
		m_nLastTime = m_nLoopStartTime = m_nStartTime = ccCurrentTime();
	}

	__int64 currentTime = ccCurrentTime();
	long runTime = long(currentTime - m_nStartTime);
	long loopTime = long(currentTime - m_nLoopStartTime);
	long during = runTime;
	long frameTime = (long)(currentTime - m_nLastTime);

	m_nLastTime = currentTime;
	if (m_nAniLoopTime > 0)
	{
		if (m_nEffectDuring < 0)
		{
			m_nEffectDuring = m_nAniLoopTime;
		}

		during = during%m_nAniLoopTime;
	}

	//end animation?
	if (m_nEffectDuring > 0 && runTime >= m_nEffectDuring)
	{
		m_effectStatus = SES_IDLE;
		if (m_pEffectCallback != NULL)
		{
			(*m_pEffectCallback)(EE_FINISHED, m_lpParam);
		}
		return;
	}

	//if (faceRes == NULL || faceRes->numFace <= 0)return;

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

	cocos2d::Vector<cocos2d::MeshRenderCmd *> m_renderCMD;
	//更新cocos动画信息
	{
		vector<FSObject *>::iterator it = m_effectInfo.m_cocosScene.m_vRootObjects.begin();
		while (it != m_effectInfo.m_cocosScene.m_vRootObjects.end())
		{
			FSObject *pObject = (*it);
			pObject->update(frameTime*0.001);
			if (pObject != NULL)
			{
				pObject->visit(m_renderCMD, m_effectInfo.m_cocosScene.m_vLights.size() > 0);

				if (pObject == m_effectInfo.m_cocosScene.m_pFacialObj && faceRes->numFace > 0)
				{
					float rotate[] = { faceRes->arrFace[0].pitch, faceRes->arrFace[0].yaw, faceRes->arrFace[0].roll };
					m_effectInfo.m_cocosScene.m_pFacialObj->updateNeckRotate(rotate);
				}
			}

			++it;
		}
		std::sort(m_renderCMD.begin(), m_renderCMD.end(), cocos2d::MeshRenderCmdCmp());
	}

	vector<FaceInfoSort> vFaceSort;
	for (int i = 0; i < faceRes->numFace; ++i)
	{
		FaceInfoSort faceSort;
		faceSort.zDepth = faceRes->arrFace[i].z;
		faceSort.index = i;

		vFaceSort.push_back(faceSort);
	}
	std::sort(vFaceSort.begin(), vFaceSort.end(), compare_face_depth);
	for (int k = 0; k < vFaceSort.size(); ++k)
	{
		ccFDFaceRes *faceResT = (ccFDFaceRes *)faceRes;
		int faceIndex = vFaceSort[k].index;
		ccFDShape68 &faceInfo = faceResT->arrFace[faceIndex];
		Mat4 matRotateX;
		Mat4 matRotateY;
		Mat4 matRotateZ;
		Mat4 matTranslate;
		Mat4 matRotateXYZ;

		Mat4::createRotationX(-faceRes->arrFace[faceIndex].pitch / 180.0f*PI, &matRotateX);
		Mat4::createRotationY(-faceRes->arrFace[faceIndex].yaw / 180.0f*PI, &matRotateY);
		Mat4::createRotationZ(faceRes->arrFace[faceIndex].roll / 180.0f*PI, &matRotateZ);

		Mat4::multiply(matRotateX, matRotateY, &matRotateXYZ);
		Mat4::multiply(matRotateXYZ, matRotateZ, &matRotateXYZ);
		Mat4::createTranslation(Vec3(faceRes->arrFace[faceIndex].x, faceRes->arrFace[faceIndex].y, -faceRes->arrFace[faceIndex].z), &matTranslate);
		Mat4::multiply(matTranslate, matRotateXYZ, &matRotateXYZ);
		memcpy(faceInfo.arrWMatrix, (float *)(&matRotateXYZ), sizeof(Mat4));
	}

	float blendFactor[] = { 0.f,0.f,0.f,0.f };
	//绘制背景动画信息
	if(pExtInfo != NULL)
	{
		DeviceContextPtr->OMSetBlendState(m_pBSEnable, blendFactor, 0xffffffff);
		DeviceContextPtr->OMSetDepthStencilState(m_pDepthStateDisable, 0);
		render2DEffect(width, height, during, BTLayer_Func);
		
		struct SMaskInfo
		{
			unsigned char type;
			unsigned char *pMask;
			int maskW;
			int maskH;
		};
		SMaskInfo *info = (SMaskInfo *)pExtInfo;
		if(info->type == 1 && m_effectInfo.m_v2DBGAnimation != NULL)
		{
			float arrClip[] = { 0,0,1,1 };
			ID3D11ShaderResourceView *pBGRV = m_effectInfo.m_v2DBGAnimation->GetSRV(during);
			int dw, dh;
			m_effectInfo.m_v2DBGAnimation->getSize(dw, dh);

			if (dw > 0 && dh > 0)
			{
				float fImgRatio = 1.0f*dw / dh;
				float fRTRatio = 1.0f * width / height;
				if (fImgRatio > fRTRatio)
				{
					float fClipX = fRTRatio / fImgRatio;
					fClipX = 1.0f - fClipX;
					arrClip[0] = fClipX*0.5f;
					arrClip[2] = 1.0f - fClipX*0.5f;
				}
				else
				{
					fImgRatio = 1.0f*dh / dw;
					fRTRatio = 1.0f*height / width;
					float fClipY = fRTRatio / fImgRatio;
					fClipY = 1.0f - fClipY;
					arrClip[1] = fClipY*0.5f;
					arrClip[3] = 1.0f - fClipY*0.5f;
				}
			}

			m_rectDraw->render(pBGRV, arrClip, info->pMask, info->maskW, info->maskH);
		}
		else if (info->type == 2 && m_effectInfo.m_cocosScene.m_pFacialObj != NULL)
		{
			#define AVANTAR_TYPE 2
			struct SAvantarInfo
			{
				unsigned char type;
				float arrAlpha[51];
			};

			SAvantarInfo *info = (SAvantarInfo *)pExtInfo;
			m_effectInfo.m_cocosScene.m_pFacialObj->updateFacialInfo(info->arrAlpha);
		}
		else
		{
			DeviceContextPtr->ClearDepthStencilView(m_pFBO->getDepthStencilView(), D3D11_CLEAR_DEPTH, 1.0f, 0);
			render3DSEffect(width, height, during, BTLayer_Func, faceRes);
			D3D11_VIEWPORT vp;
			vp.Width = (FLOAT)width;
			vp.Height = (FLOAT)height;
			vp.MinDepth = 0.0f;
			vp.MaxDepth = 1.0f;
			vp.TopLeftX = 0;
			vp.TopLeftY = 0;
			DeviceContextPtr->RSSetViewports(1, &vp);

			float arrClip[] = { 0,0,1,1 };
			m_rectDraw->renderAlpha(arrClip, info->pMask, info->maskW, info->maskH, width, height);
		}
	}


	if (faceRes != NULL && faceRes->numFace > 0)
	{


		//渲染人头像信息,用于后续剔除(禁止混合，禁止写入颜色buffer，开启深度测试和深度buffer写)
		DeviceContextPtr->ClearDepthStencilView(m_pFBO->getDepthStencilView(), D3D11_CLEAR_DEPTH, 1.0f, 0);

		//绘制不被人头模型遮挡的效果
		render3DSEffect(width, height, during, TPLayer_Func, faceRes);

		//绘制人头模型，用于遮挡剔除
		DeviceContextPtr->OMSetDepthStencilState(m_pDepthStateEnable, 0);
		DeviceContextPtr->OMSetBlendState(m_pBSDisableWriteDisable, blendFactor, 0xffffffff);
		m_pShaderForHeaderCull->useShader();
		for (int k = 0; k < vFaceSort.size(); ++k)
		{
			//构建人脸变换矩阵信息
			int faceIndex = vFaceSort[k].index;
			const ccFDShape68 &faceInfo = faceRes->arrFace[faceIndex];
			Mat4 matRotateXYZ = Mat4(faceInfo.arrWMatrix);

			//构建人脸变换矩阵信息
			float fAngle = atan(0.5) * 2;
			int faceL = faceRes->arrFace[faceIndex].gesture_box_left;
			int faceT = faceRes->arrFace[faceIndex].gesture_box_top;
			int faceW = faceRes->arrFace[faceIndex].gesture_box_right - faceL;
			int faceH = faceRes->arrFace[faceIndex].gesture_box_bottom- faceT;
			if (faceH > faceW)
			{
				fAngle = atan(0.5*faceH / faceW) * 2;
			}
			XMMATRIX matProjDX = XMMatrixPerspectiveFovRH(fAngle, faceW*1.0f / faceH, 1, 2000);
			matProjDX = XMMatrixTranspose(matProjDX);

			D3D11_VIEWPORT vp;
			vp.Width = (FLOAT)faceW;
			vp.Height = (FLOAT)faceH;
			vp.MinDepth = 0.0f;
			vp.MaxDepth = 1.0f;
			vp.TopLeftX = faceL;
			vp.TopLeftY = faceT;
			DeviceContextPtr->RSSetViewports(1, &vp);


			Mat4 matProj;
			memcpy(&matProj, &matProjDX, sizeof(XMMATRIX));
			Mat4::multiply(matProj, matRotateXYZ, &matRotateXYZ);

			HeaderForCullConstantBuffer matWVP;
			matWVP.mWVP = matRotateXYZ;
			for (int i = 0; i<m_vHeaderModel.size(); ++i)
			{
				HeaderModel &model = m_vHeaderModel[i];

				//设置矩阵变换
				DeviceContextPtr->UpdateSubresource(m_pConstantBufferForHeaderCull, 0, NULL, &matWVP, 0, 0);
				DeviceContextPtr->VSSetConstantBuffers(0, 1, &m_pConstantBufferForHeaderCull);

				//设置顶点数据
				unsigned int nStride = sizeof(vec3);
				unsigned int nOffset = 0;
				DeviceContextPtr->IASetVertexBuffers(0, 1, &model.m_headerVerticeBuffer, &nStride, &nOffset);
				DeviceContextPtr->IASetIndexBuffer(model.m_headerIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

				DeviceContextPtr->DrawIndexed(model.m_nFaces * 3, 0, 0);
			}
		}

		//绘制被人脸遮挡的效果,暂时cocos 3d模型绘制只支持被人脸遮挡绘制
		render3DSEffect(width, height, during, CTLayer_Func, faceRes, &m_renderCMD);
	}

	D3D11_VIEWPORT vp;
	vp.Width = (FLOAT)width;
	vp.Height = (FLOAT)height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	DeviceContextPtr->RSSetViewports(1, &vp);

	render2DEffect(width, height, during, CTPLayer_Func);

	DeviceContextPtr->OMSetBlendState(m_pBSDisable, blendFactor, 0xffffffff);
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

	if (m_nAniLoopTime > 0 && m_nEffectDuring < 0)
	{
		m_nEffectDuring = m_nAniLoopTime;
	}

	if (m_nAniLoopTime > 0)
	{
		//loop animations
		if (loopTime >= m_nAniLoopTime)
		{
			if (m_pEffectCallback != NULL && m_nAniLoopTime != m_nEffectDuring)
			{
				(*m_pEffectCallback)(EE_LOOP, m_lpParam);
			}

			m_nLoopStartTime = currentTime + (loopTime - m_nAniLoopTime);
		}
	}
}

bool StickerEffect::prepare()
{
	if (m_vHeaderModel.size() == 0)
	{
		HeaderModel headerModel;
		headerModel.m_arrIndex = g_headerModelForCullIndces;
		headerModel.m_arrPos = (vec3 *)g_headerModelForCullVertices;
		headerModel.m_nFaces = g_headerModelForCullFaceCount;
		headerModel.m_nVertex = g_headerModelForCullVerticesCount;

		headerModel.updateGpuBuffer();
		m_vHeaderModel.push_back(headerModel);
	}

	if (m_bPrepare)return true;
	m_bPrepare = true;

	//创建纹理采样
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	HRESULT hr = DevicePtr->CreateSamplerState(&sampDesc, &m_pSamplerLinear);
	if (FAILED(hr))return false;

	{
		//创建shader
		m_pShader = new DX11Shader();
		m_pShader->initShaderWithString(s_szStickerShader);
		//m_pShader->initShaderWithFile("BaseTexture.fx");

		//创建constbuffer 参数
		D3D11_BUFFER_DESC bd;
		memset(&bd, 0, sizeof(bd));
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(StickerConstantBuffer);
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bd.CPUAccessFlags = 0;
		hr = DevicePtr->CreateBuffer(&bd, NULL, &m_pConstantBuffer);
		if (FAILED(hr))return false;
	}
	
	//创建
	{
		m_pShaderForHeaderCull = new DX11ShaderForHeaderCull();
		m_pShaderForHeaderCull->initShaderWithString(s_szHeaderForCullShader);

		//创建constbuffer 参数
		D3D11_BUFFER_DESC bd;
		memset(&bd, 0, sizeof(bd));
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(HeaderForCullConstantBuffer);
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bd.CPUAccessFlags = 0;
		hr = DevicePtr->CreateBuffer(&bd, NULL, &m_pConstantBufferForHeaderCull);
	}

	//混合状态
	if (m_pBSEnable == NULL)
	{
		m_pBSEnable = ContextInst->fetchBlendState(true, false, true);
	}
	if (m_pBSWriteDisable == NULL)
	{
		m_pBSWriteDisable = ContextInst->fetchBlendState(false, false, false);
	}
	if (m_pBSDisable == NULL)
	{
		m_pBSDisable = ContextInst->fetchBlendState(false, false, true);
	}
	if (m_pBSDisableWriteDisable == NULL)
	{
		m_pBSDisableWriteDisable = ContextInst->fetchBlendState(false, false, false);
	}


	//深度状态
	if (m_pDepthStateEnable == NULL)
	{
		m_pDepthStateEnable = ContextInst->fetchDepthStencilState(true, true);
	}
	if (m_pDepthStateDisable == NULL)
	{
		m_pDepthStateDisable = ContextInst->fetchDepthStencilState(false, false);
	}
	if (m_pDepthStateEnableWriteDisable == NULL)
	{
		m_pDepthStateEnableWriteDisable = ContextInst->fetchDepthStencilState(true, false);
	}
	return true;
}

ID3D11ShaderResourceView *StickerEffect::renderEffectToTexture(ID3D11ShaderResourceView *pTexture, int width, int height, const ccFDFaceRes *faceRes)
{
	assert(width > 0);
	assert(width > 0);
	prepare();
	touchThreadSource();
	if (m_pFBO == NULL || m_pFBO->width() != width || m_pFBO->height() != height)
	{
		if (m_pFBO != NULL)
		{
			delete m_pFBO;
		}

		m_pFBO = new DX11FBO();
		m_pFBO->initWithTexture(width, height, true);
	}
	if (m_rectDraw == NULL)
	{
		m_rectDraw = new RectDraw();
		m_rectDraw->init(1, 1);
	}

	float blendFactor[] = { 0.f,0.f,0.f,0.f };
	DeviceContextPtr->OMSetBlendState(m_pBSEnable, blendFactor, 0xffffffff);

	m_rectDraw->setShaderTextureView(pTexture);

	m_pFBO->bind();
	//渲染背景
	m_rectDraw->render();

	//渲染贴纸效果
	renderEffect(width, height, faceRes);

	return m_pFBO->getTexture()->getTexShaderView();
}

bool StickerEffect::renderEffectToTexture(ID3D11ShaderResourceView *pInputTexture, ID3D11Texture2D *pTargetTexture, int width, int height, const ccFDFaceRes *faceRes, void *pExtInfo)
{
	if (m_effectStatus == SES_IDLE || m_effectStatus == SES_RUN)
	{
		//释放autopool对象
		cocos2d::PoolManager::getInstance()->getCurrentPool()->clear();
	}

	assert(width > 0);
	assert(width > 0);
	prepare();
	touchThreadSource();

	D3D11_TEXTURE2D_DESC texDesc;
	pTargetTexture->GetDesc(&texDesc);

	if (m_pFBO == NULL || m_pFBO->width() != width || m_pFBO->height() != height || m_pTargetTexture == NULL)
	{
		if (m_pFBO != NULL)
		{
			delete m_pFBO;
		}

		SAFEDEL(m_pTargetTexture);

		m_pTargetTexture = new DX11Texture();
		m_pTargetTexture->initTexture(texDesc.Format, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET, width, height, NULL, 0, false, true);

		m_pFBO = new DX11FBO();
		m_pFBO->initWithTexture(width, height, true, m_pTargetTexture->getTex());
	}
	if (m_rectDraw == NULL)
	{
		m_rectDraw = new RectDraw();
		m_rectDraw->init(1, 1);
	}

	if (!m_effectInfo.hasObject() || m_effectStatus == SES_IDLE)
	{
		return false;
	}

	float blendFactor[] = { 0.f,0.f,0.f,0.f };
	DeviceContextPtr->OMSetBlendState(m_pBSEnable, blendFactor, 0xffffffff);

	unsigned char *pExtInfoBytes = (unsigned char *)pExtInfo;
	if (pExtInfoBytes != NULL && pExtInfoBytes[0] <= 0)
	{
		pExtInfo = NULL;
	}

	m_pFBO->bind();
	//渲染背景
	m_rectDraw->setShaderTextureView(pInputTexture);

	int nBGEffect = 0;
	for (int i = 0; i < m_effectInfo.m_v2DEffectModel.size(); ++i)
	{
		if (m_effectInfo.m_v2DEffectModel[i].m_Layer < 0)
		{
			++nBGEffect;
		}
	}
	if (false)//pExtInfo != NULL && (nBGEffect != 0 || m_effectInfo.m_v2DBGAnimation != NULL))
	{
	}
	else
	{
		m_rectDraw->renderOpaque();
	}

	//渲染贴纸效果
	renderEffect(width, height, faceRes, pExtInfo);



	unsigned int sub = D3D11CalcSubresource(0, 0, 1);
	DeviceContextPtr->ResolveSubresource(
		pTargetTexture,
		sub,
		m_pTargetTexture->getTex(),
		sub,
		texDesc.Format
	);
	return (m_effectStatus == SES_RUN);
}

void StickerEffect::render2DEffect(int width, int height, long during, LayerRenderFunc fRender)
{
	//绘制2D贴图效果
	if (m_effectInfo.m_v2DEffectModel.size() > 0)
	{
		m_pShader->useShader();
	}
	StickerConstantBuffer matWVP;
	matWVP.mWVP = Mat4::IDENTITY;
	for (int i = 0; i<m_effectInfo.m_v2DEffectModel.size(); ++i)
	{
		Effect2DRect &rectEffect = m_effectInfo.m_v2DEffectModel[i];

		if ((*fRender)(rectEffect.m_Layer))
		{
			rectEffect.updateRenderInfo(width, height);

			unsigned int nStride = sizeof(EffectVertex);
			unsigned int nOffset = 0;

			//设置矩阵变换
			matWVP.mColor = XMVectorSet(rectEffect.m_fMixColor.x, rectEffect.m_fMixColor.y, rectEffect.m_fMixColor.z, rectEffect.m_fMixColor.w);
			DeviceContextPtr->UpdateSubresource(m_pConstantBuffer, 0, NULL, &matWVP, 0, 0);
			DeviceContextPtr->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);

			//设置纹理以及纹理采样
			ID3D11ShaderResourceView *pMyShaderResourceView = rectEffect.m_drawable->GetSRV(during);
			DeviceContextPtr->PSSetShaderResources(0, 1, &pMyShaderResourceView);
			DeviceContextPtr->PSSetSamplers(0, 1, &m_pSamplerLinear);

			//设置顶点数据
			DeviceContextPtr->IASetVertexBuffers(0, 1, &rectEffect.m_rectVerticeBuffer, &nStride, &nOffset);
			DeviceContextPtr->IASetIndexBuffer(rectEffect.m_rectIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

			DeviceContextPtr->DrawIndexed(2 * 3, 0, 0);
		}
	}
}


void StickerEffect::render3DSEffect(int width, int height, long during, LayerRenderFunc fRender, const ccFDFaceRes *faceRes, void *pExtInfo)
{
	cocos2d::Vector<cocos2d::MeshRenderCmd *> renderCMDT;
	cocos2d::Vector<cocos2d::MeshRenderCmd *> *pRenderCMDT = &renderCMDT;
	if (pExtInfo != NULL)
	{
		pRenderCMDT = (cocos2d::Vector<cocos2d::MeshRenderCmd *> *)pExtInfo;
	}

	cocos2d::Vector<cocos2d::MeshRenderCmd *> &m_renderCMD = *pRenderCMDT;
	if (faceRes != NULL && faceRes->numFace > 0)
	{
		vector<FaceInfoSort> vFaceSort;
		for (int i = 0; i < faceRes->numFace; ++i)
		{
			FaceInfoSort faceSort;
			faceSort.zDepth = faceRes->arrFace[i].z;
			faceSort.index = i;

			vFaceSort.push_back(faceSort);
		}
		std::sort(vFaceSort.begin(), vFaceSort.end(), compare_face_depth);
		//绘制不透明的物体
		float blendFactor[] = { 0.f,0.f,0.f,0.f };
		DeviceContextPtr->OMSetBlendState(m_pBSDisable, blendFactor, 0xffffffff);
		m_pShader->useShader();
		for (int k = 0; k < vFaceSort.size(); ++k)
		{
			//构建人脸变换矩阵信息
			int faceIndex = vFaceSort[k].index;
			const ccFDShape68 &faceInfo = faceRes->arrFace[faceIndex];
			//构建人脸变换矩阵信息
			float fAngle = atan(0.5) * 2;
			int faceL = faceRes->arrFace[faceIndex].gesture_box_left;
			int faceT = faceRes->arrFace[faceIndex].gesture_box_top;
			int faceW = faceRes->arrFace[faceIndex].gesture_box_right - faceL;
			int faceH = faceRes->arrFace[faceIndex].gesture_box_bottom - faceT;
			if (faceH > faceW)
			{
				fAngle = atan(0.5*faceH / faceW) * 2;
			}
			XMMATRIX matProjDX = XMMatrixPerspectiveFovRH(fAngle, faceW*1.0f / faceH, 1, 2000);
			matProjDX = XMMatrixTranspose(matProjDX);

			D3D11_VIEWPORT vp;
			vp.Width = (FLOAT)faceW;
			vp.Height = (FLOAT)faceH;
			vp.MinDepth = 0.0f;
			vp.MaxDepth = 1.0f;
			vp.TopLeftX = faceL;
			vp.TopLeftY = faceT;
			DeviceContextPtr->RSSetViewports(1, &vp);


			Mat4 matRotateXYZ = Mat4(faceInfo.arrWMatrix);

			Mat4 matProj;
			memcpy(&matProj, &matProjDX, sizeof(XMMATRIX));
			Mat4::multiply(matProj, matRotateXYZ, &matRotateXYZ);
			StickerConstantBuffer matWVP;
			matWVP.mWVP = matRotateXYZ;
			for (int i = 0; i<m_effectInfo.m_vEffectModel.size(); ++i)
			{
				EffectModel &model = m_effectInfo.m_vEffectModel[i];
				if (!model.m_bTransparent && (*fRender)(model.m_Layer))
				{
					matWVP.mColor = XMVectorSet(model.m_fMixColor.x, model.m_fMixColor.y, model.m_fMixColor.z, model.m_fMixColor.w);

					//设置矩阵变换
					DeviceContextPtr->UpdateSubresource(m_pConstantBuffer, 0, NULL, &matWVP, 0, 0);
					DeviceContextPtr->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);

					//设置纹理以及纹理采样
					ID3D11ShaderResourceView *pMyShaderResourceView = model.m_drawable->GetSRV(during);
					DeviceContextPtr->PSSetShaderResources(0, 1, &pMyShaderResourceView);
					DeviceContextPtr->PSSetSamplers(0, 1, &m_pSamplerLinear);

					//设置顶点数据
					unsigned int nStride = sizeof(EffectVertex);
					unsigned int nOffset = 0;
					DeviceContextPtr->IASetVertexBuffers(0, 1, &model.m_rectVerticeBuffer, &nStride, &nOffset);
					DeviceContextPtr->IASetIndexBuffer(model.m_rectIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

					DeviceContextPtr->DrawIndexed(model.m_nFaces * 3, 0, 0);
				}
			}
		}
		for (int k = 0; k < vFaceSort.size(); ++k)
		{
			int faceIndex = vFaceSort[k].index;
			const ccFDShape68 &faceInfo = faceRes->arrFace[faceIndex];
			Mat4 matRotateXYZ = Mat4(faceInfo.arrWMatrix);

			//构建人脸变换矩阵信息
			float fAngle = atan(0.5) * 2;
			int faceL = faceRes->arrFace[faceIndex].gesture_box_left;
			int faceT = faceRes->arrFace[faceIndex].gesture_box_top;
			int faceW = faceRes->arrFace[faceIndex].gesture_box_right - faceL;
			int faceH = faceRes->arrFace[faceIndex].gesture_box_bottom - faceT;
			if (faceH > faceW)
			{
				fAngle = atan(0.5*faceH / faceW) * 2;
			}
			XMMATRIX matProjDX = XMMatrixPerspectiveFovRH(fAngle, faceW*1.0f / faceH, 1, 2000);
			matProjDX = XMMatrixTranspose(matProjDX);

			D3D11_VIEWPORT vp;
			vp.Width = (FLOAT)faceW;
			vp.Height = (FLOAT)faceH;
			vp.MinDepth = 0.0f;
			vp.MaxDepth = 1.0f;
			vp.TopLeftX = faceL;
			vp.TopLeftY = faceT;
			DeviceContextPtr->RSSetViewports(1, &vp);
			Mat4 matProj;
			memcpy(&matProj, &matProjDX, sizeof(XMMATRIX));
			Mat4::multiply(matProj, matRotateXYZ, &matRotateXYZ);


			for (size_t i = 0; i < m_renderCMD.size(); ++i)
			{
				Mat4 matWorld = matRotateXYZ;
				const Mat4 &matInit = m_renderCMD.at(i)->getInitWTransform();
				Mat4::multiply(matWorld, matInit, &matWorld);
				m_renderCMD.at(i)->updateTransform(matWorld);
			}
			//绘制cocos模型透明物体
			Mat4 matView = Mat4::IDENTITY;
			std::vector<BaseLight> arrLights;
			{
				for (size_t i = 0; i < m_renderCMD.size(); ++i)
				{
					if (!m_renderCMD.at(i)->isTransparent())
						m_renderCMD.at(i)->render(matView, matProj, m_effectInfo.m_cocosScene.m_vLights);
				}
			}
		}

		//绘制透明模型效果
		DeviceContextPtr->OMSetBlendState(m_pBSEnable, blendFactor, 0xffffffff);
		DeviceContextPtr->OMSetDepthStencilState(m_pDepthStateEnableWriteDisable, 0);
		m_pShader->useShader();
		for (int k = 0; k < vFaceSort.size(); ++k)
		{
			int faceIndex = vFaceSort[k].index;
			const ccFDShape68 &faceInfo = faceRes->arrFace[faceIndex];
			Mat4 matRotateXYZ = Mat4(faceInfo.arrWMatrix);

			//构建人脸变换矩阵信息
			float fAngle = atan(0.5) * 2;


			int faceL = faceRes->arrFace[faceIndex].gesture_box_left;
			int faceT = faceRes->arrFace[faceIndex].gesture_box_top;
			int faceW = faceRes->arrFace[faceIndex].gesture_box_right - faceL;
			int faceH = faceRes->arrFace[faceIndex].gesture_box_bottom - faceT;

		    faceL -=  faceW;
			faceT -= faceH;
			faceW *= 3;
			faceH *= 3;

			if (faceH > faceW)
			{
				fAngle = atan(0.5*faceH / faceW) * 2;
			}
			XMMATRIX matProjDX = XMMatrixPerspectiveFovRH(fAngle, faceW*1.0f / faceH, 1, 2000);
			matProjDX = XMMatrixTranspose(matProjDX);

			D3D11_VIEWPORT vp;
			vp.Width = (FLOAT)faceW;
			vp.Height = (FLOAT)faceH;
			vp.MinDepth = 0.0f;
			vp.MaxDepth = 1.0f;
			vp.TopLeftX = faceL;
			vp.TopLeftY = faceT;
			DeviceContextPtr->RSSetViewports(1, &vp);

			Mat4 matProj;
			memcpy(&matProj, &matProjDX, sizeof(XMMATRIX));
			Mat4::multiply(matProj, matRotateXYZ, &matRotateXYZ);
			Vec3 scale(1.0f / 3, 1.0f / 3, 1.0f);
			Mat4::createScale(scale, &matProj);
			Mat4::multiply(matProj, matRotateXYZ, &matRotateXYZ);

			StickerConstantBuffer matWVP;
			matWVP.mWVP = matRotateXYZ;
			for (int i = 0; i<m_effectInfo.m_vEffectModel.size(); ++i)
			{
				EffectModel &model = m_effectInfo.m_vEffectModel[i];
				if (model.m_bTransparent && (*fRender)(model.m_Layer))
				{
					matWVP.mColor = XMVectorSet(model.m_fMixColor.x, model.m_fMixColor.y, model.m_fMixColor.z, model.m_fMixColor.w);

					//设置矩阵变换
					DeviceContextPtr->UpdateSubresource(m_pConstantBuffer, 0, NULL, &matWVP, 0, 0);
					DeviceContextPtr->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);

					//设置纹理以及纹理采样
					ID3D11ShaderResourceView *pMyShaderResourceView = model.m_drawable->GetSRV(during);
					DeviceContextPtr->PSSetShaderResources(0, 1, &pMyShaderResourceView);
					DeviceContextPtr->PSSetSamplers(0, 1, &m_pSamplerLinear);

					//设置顶点数据
					unsigned int nStride = sizeof(EffectVertex);
					unsigned int nOffset = 0;
					DeviceContextPtr->IASetVertexBuffers(0, 1, &model.m_rectVerticeBuffer, &nStride, &nOffset);
					DeviceContextPtr->IASetIndexBuffer(model.m_rectIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

					ContextInst->setCullMode(D3D11_CULL_FRONT);
					DeviceContextPtr->DrawIndexed(model.m_nFaces * 3, 0, 0);

					ContextInst->setCullMode(D3D11_CULL_BACK);
					DeviceContextPtr->DrawIndexed(model.m_nFaces * 3, 0, 0);
				}
			}
		}

		for (int k = 0; k < vFaceSort.size(); ++k)
		{
			int faceIndex = vFaceSort[k].index;
			const ccFDShape68 &faceInfo = faceRes->arrFace[faceIndex];

			//构建人脸变换矩阵信息
			float fAngle = atan(0.5) * 2;
			int faceL = faceRes->arrFace[faceIndex].gesture_box_left;
			int faceT = faceRes->arrFace[faceIndex].gesture_box_top;
			int faceW = faceRes->arrFace[faceIndex].gesture_box_right - faceL;
			int faceH = faceRes->arrFace[faceIndex].gesture_box_bottom - faceT;
			if (faceH > faceW)
			{
				fAngle = atan(0.5*faceH / faceW) * 2;
			}
			XMMATRIX matProjDX = XMMatrixPerspectiveFovRH(fAngle, faceW*1.0f / faceH, 1, 2000);
			matProjDX = XMMatrixTranspose(matProjDX);

			D3D11_VIEWPORT vp;
			vp.Width = (FLOAT)faceW;
			vp.Height = (FLOAT)faceH;
			vp.MinDepth = 0.0f;
			vp.MaxDepth = 1.0f;
			vp.TopLeftX = faceL;
			vp.TopLeftY = faceT;
			DeviceContextPtr->RSSetViewports(1, &vp);
			Mat4 matProj;
			memcpy(&matProj, &matProjDX, sizeof(XMMATRIX));


			Mat4 matRotateXYZ = Mat4(faceInfo.arrWMatrix);
			Mat4::multiply(matProj, matRotateXYZ, &matRotateXYZ);
			for (size_t i = 0; i < m_renderCMD.size(); ++i)
			{
				Mat4 matWorld = matRotateXYZ;
				const Mat4 &matInit = m_renderCMD.at(i)->getInitWTransform();
				Mat4::multiply(matWorld, matInit, &matWorld);
				m_renderCMD.at(i)->updateTransform(matWorld);
			}
			//绘制cocos模型透明物体
			Mat4 matView = Mat4::IDENTITY;
			std::vector<BaseLight> arrLights;
			{
				for (size_t i = 0; i < m_renderCMD.size(); ++i)
				{
					if (m_renderCMD.at(i)->isTransparent())
						m_renderCMD.at(i)->render(matView, matProj, m_effectInfo.m_cocosScene.m_vLights);
				}
			}
		}

		DeviceContextPtr->OMSetDepthStencilState(m_pDepthStateDisable, 0);
		ContextInst->setCullMode(D3D11_CULL_NONE);
	}
}