#include "C2DPicTextureEffect.h"
#include "Toolbox/Helper.h"

#include "math\Mat4.h"
#include "Toolbox/DXUtils/DXUtils.h"
#include "Toolbox/DXUtils/DX11Context.h"
#include "Toolbox/RectDraw.h"
#include "Toolbox/EffectModel.hpp"
#include "Toolbox/FSObject.h"
#include "Toolbox/FacialObject.h"
#include "Toolbox\fileSystem.h"
#include "EffectKernel/ShaderProgramManager.h"
#include "../ResourceManager.h"
#include "../FileManager.h"


C2DPicTextureEffect::C2DPicTextureEffect()
{
	m_EffectPart = FORE_EFFECT_STICKER;
	m_alpha = 1.0;
}

C2DPicTextureEffect::~C2DPicTextureEffect() {
	Release();
}

void* C2DPicTextureEffect::Clone(){
	auto tmp = new C2DPicTextureEffect();
	tmp->m_v2DEffectModel = m_v2DEffectModel;
	return tmp;
}

bool C2DPicTextureEffect::ReadConfig(XMLNode& childNode, HZIP hZip, char* pFilePath) {
	CEffectPart::ReadConfig(childNode, hZip, pFilePath);
	ReadConfig(childNode, hZip, pFilePath, "");
	return true;
}

bool C2DPicTextureEffect::ReadConfig(XMLNode & childNode, const std::string & path)
{
	CEffectPart::ReadConfig(childNode, path);
	ReadConfig(childNode, 0, NULL, path);
	return true;
}

bool C2DPicTextureEffect::Prepare() {

		string path = m_resourcePath + "/Shader/face2dBlendMakeUp.fx";
		m_pShader = ShaderProgramManager::GetInstance()->GetOrCreateShaderByPathAndAttribs(path, nullptr, 0);


		m_pConstantBuffer = DXUtils::CreateConstantBuffer(sizeof(float) * 4);

	return true;
}

void C2DPicTextureEffect::Render(BaseRenderParam& RenderParam) {

	long runTime = GetRunTime();
	if (runTime<0)
	{
		return;
	}

	m_v2DEffectModel.updateRenderInfo2(RenderParam.GetWidth(), RenderParam.GetHeight());
	auto pDoubleBuffer = RenderParam.GetDoubleBuffer();
	pDoubleBuffer->SyncAToBRegion((float*)m_v2DEffectModel.m_arrVertices, 4, 5, 1);

	m_pShader->useShader();
	
	pDoubleBuffer->BindFBOA();

	float pParam[4];
	pParam[0] = m_alpha;
	pParam[1] = GetBlendParm(m_BlendType);
	pParam[2] = 0.5;

	unsigned int nStride = sizeof(EffectVertex);
	unsigned int nOffset = 0;

	//设置矩阵变换
	DeviceContextPtr->UpdateSubresource(m_pConstantBuffer, 0, NULL, pParam, 0, 0);
	DeviceContextPtr->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
	DeviceContextPtr->PSSetConstantBuffers(0, 1, &m_pConstantBuffer);
	//设置纹理以及纹理采样

	Image* img = ResourceManager::Instance().getAnimFrame(m_anim_id, float(runTime));
	auto pMyShaderResourceView = img->tex->getTexShaderView();
	DeviceContextPtr->PSSetShaderResources(0, 1, &pMyShaderResourceView);
	auto pSrcShaderView = pDoubleBuffer->GetFBOTextureB()->getTexShaderView();
	DeviceContextPtr->PSSetShaderResources(1, 1, &pSrcShaderView);

	DeviceContextPtr->PSSetSamplers(0, 1, &m_pSamplerLinear);

	//设置顶点数据
	DeviceContextPtr->IASetVertexBuffers(0, 1, &m_v2DEffectModel.m_rectVerticeBuffer, &nStride, &nOffset);
	DeviceContextPtr->IASetIndexBuffer(m_v2DEffectModel.m_rectIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

	DeviceContextPtr->DrawIndexed(2 * 3, 0, 0);
}

void C2DPicTextureEffect::Release() {

	SAFERALEASE(m_pConstantBuffer);

	m_v2DEffectModel.destory();
}

void C2DPicTextureEffect::updateData()
{
	RectInfo info;
	info.x = (int)(m_SrcRect[0].x *1280.f);
	info.y = (int)(m_SrcRect[0].y * 720.f);
	info.width = (int)(m_SrcRect[1].x *1280.f) - info.x;
	info.height = (int)(m_SrcRect[1].y *720.f) - info.y;
	m_v2DEffectModel.setRect(info.x, info.y, info.width, info.height, alignType);
	
}


void C2DPicTextureEffect::ReadConfig(XMLNode & childNode, HZIP hZip, char * pFilePath, const std::string & path)
{
	XMLNode nodeRect = childNode.getChildNode("rect", 0);
	if(!nodeRect.isEmpty()) {
		const char* szX = nodeRect.getAttribute("x");
		const char* szY = nodeRect.getAttribute("y");
		const char* szWidth = nodeRect.getAttribute("width");
		const char* szHeight = nodeRect.getAttribute("height");
		const char* szAlignType = nodeRect.getAttribute("alignType");

		int x = atoi(szX);
		int y = atoi(szY);
		int width = atoi(szWidth);
		int height = atoi(szHeight);
		alignType = EAPT_CC;
		if (szAlignType != NULL) {
			string szArrAlignType[] = { "EAPT_LT", "EAPT_LB", "EAPT_RT", "EAPT_RB", "EAPT_CT", "EAPT_CB", "EAPT_LC", "EAPT_RC", "EAPT_CC" };
			for (int index = 0; index < EAPT_MAX; ++index) {
				if (strcmp(szAlignType, szArrAlignType[index].c_str()) == 0) {
					alignType = index;
					break;
				}
			}
		}

		m_v2DEffectModel.setRect(x, y, width, height, alignType);
	}
	XMLNode nodeRect1 = childNode.getChildNode("point", 0);
	if (!nodeRect1.isEmpty())
	{
		const char* szLeft = nodeRect1.getAttribute("left");
		const char* szUp = nodeRect1.getAttribute("up");
		const char* szRight = nodeRect1.getAttribute("right");
		const char* szDown = nodeRect1.getAttribute("down");
		const char* szAlignType = nodeRect1.getAttribute("alignType");
		if (szLeft != NULL && szUp != NULL && szRight != NULL && szDown != NULL)
		{
			float left = atof(szLeft);
			float up = atof(szUp);
			float right = atof(szRight);
			float down = atof(szDown);

			m_SrcRect[0] = Vector2(left, up);
			m_SrcRect[1] = Vector2(right, down);
			RectInfo info;
			info.x = (int)(m_SrcRect[0].x *1280.f);
			info.y = (int)(m_SrcRect[0].y * 720.f);

			info.width = (int)(m_SrcRect[1].x *1280.f) - info.x;
			info.height = (int)(m_SrcRect[1].y *720.f) - info.y;

			alignType = EAPT_CC;
			if (szAlignType != NULL) {
				string szArrAlignType[] = { "EAPT_LT", "EAPT_LB", "EAPT_RT", "EAPT_RB", "EAPT_CT", "EAPT_CB", "EAPT_LC", "EAPT_RC", "EAPT_CC" ,"EAPT_SCALE" };
				for (int index = 0; index < EAPT_MAX; ++index) {
					if (strcmp(szAlignType, szArrAlignType[index].c_str()) == 0) {
						alignType = index;
						break;
					}
				}
			}

			m_v2DEffectModel.setRect(info.x, info.y, info.width, info.height, alignType);
		}
	}

	//anidrawable 
	XMLNode nodeDrawable = childNode.getChildNode("anidrawable", 0);
	if (nodeDrawable.isEmpty())
	{
		nodeDrawable = nodeRect.getChildNode("anidrawable", 0);
	}
	if (!nodeDrawable.isEmpty())
	{
		const char *szDrawableName = nodeDrawable.getAttribute("name");

		const char *szBlendType = nodeDrawable.getAttribute("blendType");
		m_BlendType = GetBlendType(szBlendType);
		const char* szAlpha = nodeDrawable.getAttribute("alpha");
		if (szAlpha != NULL)
		{
			m_alpha = atof(szAlpha);
		}


		//get items info
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
			if (nCount >= 0 && nDuring > 0)
			{
				char szFullFile[256];
				AnimInfo anim_info;
				anim_info.fps = 1000.0 / float(nDuring);
				anim_info.dir = path;
				for (; iStart <= nCount; iStart += nStep)
				{
					sprintf(szImagePath, szItems, iStart);

					if (hZip == 0)
					{
						sprintf(szFullFile, "%s/%s", path.c_str(), szImagePath);
						anim_info.relative_filename_list.push_back(szImagePath);
					}
					else
					{
						sprintf(szFullFile, "%s/%s", path.c_str(), szImagePath);
						anim_info.relative_filename_list.push_back(szImagePath);
					}
				}
				if (hZip == 0)
				{
					m_anim_id = ResourceManager::Instance().loadAnim(anim_info);
				}
				else
				{
					m_anim_id = ResourceManager::Instance().loadAnimFromZip(anim_info, hZip);
				}
			}
		}
	}


}

bool C2DPicTextureEffect::WriteConfig(std::string & tempPath, XMLNode & root, HZIP dst, HZIP src)
{
	FileManager::Instance().SetSaveFolder(tempPath);
	Anim* anim = ResourceManager::Instance().getAnim(m_anim_id);
	AnimInfo animInfo = anim->info;
	AnimInfo renamed_info = FileManager::Instance().AddAnim(anim->info);
	animInfo.relative_filename_list = renamed_info.relative_filename_list;

	XMLNode nodeEffect = root.addChild("typeeffect");
	std::string typeEffect = "2DTexture";
	nodeEffect.addAttribute("type", typeEffect.c_str());
	nodeEffect.addAttribute("showname", m_showname.c_str());

	XMLNode nodeRect = nodeEffect.addChild("point");
	char value[100];
	sprintf(value, "%.4f", m_SrcRect[0].x);
	nodeRect.addAttribute("left", value);
	sprintf(value, "%.4f", m_SrcRect[0].y);
	nodeRect.addAttribute("up", value);
	sprintf(value, "%.4f", m_SrcRect[1].x);
	nodeRect.addAttribute("right", value);
	sprintf(value, "%.4f", m_SrcRect[1].y);
	nodeRect.addAttribute("down", value);
	sprintf(value, "%d", m_v2DEffectModel.m_nAlignType);

	string szArrAlignType[] = { "EAPT_LT", "EAPT_LB", "EAPT_RT", "EAPT_RB", "EAPT_CT", "EAPT_CB", "EAPT_LC", "EAPT_RC", "EAPT_CC" ,"EAPT_SCALE"};
	nodeRect.addAttribute("alignType", szArrAlignType[m_v2DEffectModel.m_nAlignType].c_str());


	XMLNode nodeDrwable = nodeEffect.addChild("anidrawable");
	nodeDrwable.addAttribute("name", "11");

	GetBlendParm(m_BlendType);
	nodeDrwable.addAttribute("blendType", m_BlendName.c_str());
	char alpha[256];
	sprintf(alpha, "%.4f", m_alpha);
	nodeDrwable.addAttribute("alpha", alpha);
	//get items
	if (animInfo.relative_filename_list.size() > 0)
	{
		std::string items = animInfo.relative_filename_list[0];
		items = items.replace(items.find("/") + 1, 6, "%06d");
		nodeDrwable.addAttribute("items", items.c_str());
	}
	// get iteminfo
	char iteminfo[256];
	int end = animInfo.relative_filename_list.size() - 1;
	if (animInfo.fps <= 0)
	{
		animInfo.fps = 30;
	}
	int singleFrameTime = 1000 / animInfo.fps;
	int nStep = 1;
	sprintf(iteminfo, "%d,%d,%d,%d", 0, end, singleFrameTime, nStep);
	nodeDrwable.addAttribute("iteminfo", iteminfo);
	return true;
}
