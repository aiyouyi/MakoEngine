#include "C2DPicTextureBlendEffect.h"
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
#include "Toolbox/Render/DynamicRHI.h"
#include "Toolbox/Render/TextureRHI.h"

C2DPicTextureBlendEffect::C2DPicTextureBlendEffect()
{
	m_nStartTime = -1;

}

C2DPicTextureBlendEffect::~C2DPicTextureBlendEffect() {
	Release();
}

void* C2DPicTextureBlendEffect::Clone() {
	auto tmp = new C2DPicTextureBlendEffect();
	tmp->m_v2DEffectModel = m_v2DEffectModel;
	return tmp;
}

/* xml format
<typeeffect type = "2DTexture">
	<rect name="cao" x="0" y="0" width="1280" height="312" alignType="EAPT_CB">
		<anidrawable name="cao_pc" offset="0">
			<item image="cao_pc/cao_01.png" duration="84"/>
			<item image="cao_pc/cao_02.png" duration="84"/>
			<item image="cao_pc/cao_03.png" duration="84"/>
			<item image="cao_pc/cao_04.png" duration="84"/>
			<item image="cao_pc/cao_05.png" duration="84"/>
			<item image="cao_pc/cao_06.png" duration="84"/>
			<item image="cao_pc/cao_07.png" duration="84"/>
			<item image="cao_pc/cao_08.png" duration="84"/>
			<item image="cao_pc/cao_09.png" duration="84"/>
			<item image="cao_pc/cao_10.png" duration="84"/>
		</anidrawable>
	</rect>
</typeeffect>
*/
bool C2DPicTextureBlendEffect::ReadConfig(XMLNode& childNode, HZIP hZip, char* pFilePath) {
	int i = -1;
	XMLNode nodeRect = childNode.getChildNode("rect", ++i);
	while (!nodeRect.isEmpty()) {
		const char* szX = nodeRect.getAttribute("x");
		const char* szY = nodeRect.getAttribute("y");
		const char* szWidth = nodeRect.getAttribute("width");
		const char* szHeight = nodeRect.getAttribute("height");
		const char* szAlignType = nodeRect.getAttribute("alignType");

		int x = atoi(szX);
		int y = atoi(szY);
		int width = atoi(szWidth);
		int height = atoi(szHeight);
		int alignType = EAPT_CC;
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
		m_v2DEffectModel.m_drawable = readAnidrawable(nodeRect, hZip, pFilePath);

		nodeRect = childNode.getChildNode("rect", ++i);
	}

	if (i == 0)
		return false;
	XMLNode nodeTime = childNode.getChildNode("time", 0);
	if (!nodeTime.isEmpty())
	{
		const char* szDelay = nodeTime.getAttribute("delay");
		const char* szPlay = nodeTime.getAttribute("play");
		const char* szAll = nodeTime.getAttribute("all");
		if (szDelay != NULL)
		{
			m_play.delaytime = atoi(szDelay);
		}
		if (szPlay != NULL)
		{
			m_play.playtime = atoi(szPlay);
		}

		if (szAll != NULL)
		{
			m_play.alltime = atoi(szAll);
		}


	}


	return true;
}

bool C2DPicTextureBlendEffect::WriteConfig(XMLNode& root, HZIP dst, HZIP src) {
	// 	static int order = 0;
	// 	XMLNode child = root.addChild("typeeffect");
	// 	child.addAttribute("type", "2DTexture");
	// 	{
	// 		XMLNode rect = child.addChild("rect");
	// 		rect.addAttribute("x", std::to_string(m_v2DEffectModel.m_x).c_str());
	// 		rect.addAttribute("y", std::to_string(m_v2DEffectModel.m_y).c_str());
	// 		rect.addAttribute("width", std::to_string(m_v2DEffectModel.m_width).c_str());
	// 		rect.addAttribute("height", std::to_string(m_v2DEffectModel.m_height).c_str());
	// 		if (m_v2DEffectModel.m_nAlignType >= EAPT_MAX) {
	// 			rect.addAttribute("alignType", "EAPT_MAX");
	// 		} else {
	// 			string szArrAlignType[] = { "EAPT_LT", "EAPT_LB", "EAPT_RT", "EAPT_RB", "EAPT_CT", \
	// 				"EAPT_CB", "EAPT_LC", "EAPT_RC", "EAPT_CC"};
	// 			rect.addAttribute("alignType", szArrAlignType[int(m_v2DEffectModel.m_nAlignType)].c_str());
	// 		}
	// 
	// 		AnimationDrawable* drawable = static_cast<AnimationDrawable*>(m_v2DEffectModel.m_drawable);
	// 		XMLNode drawable_xml = rect.addChild("anidrawable");
	// 		string drawable_name = "2DTexture" + std::to_string(order++);
	// 		string img_folder = getTempFolder() + "/" + drawable_name;
	// 		_mkdir(img_folder.c_str());
	// 
	// 		drawable_xml.addAttribute("name", drawable_name.c_str());
	// 		drawable_xml.addAttribute("offset", std::to_string(drawable->getOffset()).c_str());
	// 		vector<AniPair>& tex_pairs = drawable->getTexPair();
	// 		for (int j = 0; j < tex_pairs.size(); j++) {
	// 			XMLNode item = drawable_xml.addChild("item");
	// 			string item_name = drawable_name + "/" + std::to_string(j) + ".png";
	// 			item.addAttribute("image", item_name.c_str());
	// 			if(j == 0)
	// 				item.addAttribute("duration", std::to_string(tex_pairs[j].during).c_str());
	// 			else
	// 				item.addAttribute("duration", std::to_string(tex_pairs[j].during - tex_pairs[j - 1].during).c_str());
	// 			
	// // 			{
	// // 				int w = tex_pairs[j].pTex->width();
	// // 				int h = tex_pairs[j].pTex->height();
	// // 				unsigned char *buf = new unsigned char[w * h * 4];
	// // 				tex_pairs[j].pTex->ReadTextureToCpu(buf);
	// // 				cv::Mat img(h, w, CV_8UC4, buf);
	// // 				cv::cvtColor(img, img, CV_BGRA2RGBA);
	// // 				string img_name = img_folder + "/" + std::to_string(j) + ".png";
	// // 				cv::imwrite(img_name, img);
	// // 				delete buf;
	// // 			}
	// 		}
	// 	}
	return true;
}


bool C2DPicTextureBlendEffect::Prepare() {
	if (nullptr == m_pShader) {
		string path = m_resourcePath + "/Shader/picTextureBlend.fx";
		m_pShader = ShaderProgramManager::GetInstance()->GetOrCreateShaderByPathAndAttribs(path, nullptr, 0);

		m_pConstantBuffer = DXUtils::CreateConstantBuffer(sizeof(PicTextureBlendConstantBuffer));
	}
	return true;
}

void C2DPicTextureBlendEffect::Render(BaseRenderParam& RenderParam) {
	
	auto BodyTexture = RenderParam.GetBodyMaskTexture();
	if (BodyTexture == NULL)
	{
		return;
	}

	long runTime = GetRunTime();
	if (runTime < 0)
	{
		return;
	}

	float blendFactor[] = { 0.f,0.f,0.f,0.f };
	DeviceContextPtr->OMSetBlendState(m_pBlendStateNormal, blendFactor, 0xffffffff);

	auto pDoubleBuffer = RenderParam.GetDoubleBuffer();
	//pDoubleBuffer->SyncAToB();
	m_pShader->useShader();
	pDoubleBuffer->BindFBOB();

	PicTextureBlendConstantBuffer matWVP;
	matWVP.mWVP = cocos2d::Mat4::IDENTITY;

	m_v2DEffectModel.updateRenderInfo(RenderParam.GetWidth(), RenderParam.GetHeight());

	unsigned int nStride = sizeof(EffectVertex);
	unsigned int nOffset = 0;

	//设置矩阵变换
	matWVP.mColor = XMVectorSet(m_v2DEffectModel.m_fMixColor.x, m_v2DEffectModel.m_fMixColor.y, m_v2DEffectModel.m_fMixColor.z, m_v2DEffectModel.m_fMixColor.w);
	DeviceContextPtr->UpdateSubresource(m_pConstantBuffer, 0, NULL, &matWVP, 0, 0);
	DeviceContextPtr->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);

	//设置纹理以及纹理采样
	ID3D11ShaderResourceView* pMyShaderResourceView = m_v2DEffectModel.m_drawable->GetSRV(runTime);
	auto pSrcShaderView = pDoubleBuffer->GetFBOTextureA()->getTexShaderView();
	auto pMaskView = BodyTexture->getTexShaderView();
	DeviceContextPtr->PSSetShaderResources(0, 1, &pMyShaderResourceView);
	DeviceContextPtr->PSSetShaderResources(1, 1, &pSrcShaderView);
	DeviceContextPtr->PSSetShaderResources(2, 1, &pMaskView);
	DeviceContextPtr->PSSetSamplers(0, 1, &m_pSamplerLinear);

	//设置顶点数据
	DeviceContextPtr->IASetVertexBuffers(0, 1, &m_v2DEffectModel.m_rectVerticeBuffer, &nStride, &nOffset);
	DeviceContextPtr->IASetIndexBuffer(m_v2DEffectModel.m_rectIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

	DeviceContextPtr->DrawIndexed(2 * 3, 0, 0);
	pDoubleBuffer->SwapFBO();
}

void C2DPicTextureBlendEffect::Release() {

	SAFERALEASE(m_pConstantBuffer);

	m_v2DEffectModel.destory();
}

void C2DPicTextureBlendEffect::resetAnidrawable(const std::vector<std::string> & file_list) {
	//for_each(file_list.begin(), file_list.end(), [](const string& file) {std::cout << file << std::endl; });

	AnimationDrawable* drawable = new AnimationDrawable();
	drawable->setOffset(0);
	drawable->setLoopMode(ELM_REPEAT);

	for (auto file : file_list) {

		auto TexRHI = GetDynamicRHI()->CreateTextureFromFileCPUAcess(file);
		drawable->appandTex(84, TexRHI);
	}

	if (m_v2DEffectModel.m_drawable) delete m_v2DEffectModel.m_drawable;

	m_v2DEffectModel.m_drawable = drawable;
}

C2DPicTextureBlendEffect * C2DPicTextureBlendEffect::createEffect()
{
	C2DPicTextureBlendEffect* effect = new C2DPicTextureBlendEffect();
	char *path = "./BuiltInResource/createEffect/2DTexture.zip";
	CEffectPart::createEffect(path, "2DTexture", effect);
	return effect;
}

AnimationDrawable* C2DPicTextureBlendEffect::readAnidrawable(XMLNode& childNode, HZIP hZip, char* pFilePath) {
	XMLNode nodeDrawable = childNode.getChildNode("anidrawable", 0);
	if (!nodeDrawable.isEmpty()) {
		const char* szDrawableName = nodeDrawable.getAttribute("name");
		AnimationDrawable* drawable = new AnimationDrawable();

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
		while (!nodeItem.isEmpty()) {
			char szFullFile[256];
			const char* szImagePath = nodeItem.getAttribute("image");
			sprintf(szFullFile, "%s/%s", pFilePath, szImagePath);

			std::shared_ptr< CC3DTextureRHI> TexRHI = GetDynamicRHI()->FetchTexture(szFullFile, bGenMipmap);
			if (TexRHI == nullptr)
			{
				TexRHI = GetDynamicRHI()->CreateTextureFromZip(hZip, szImagePath, bGenMipmap);
				GetDynamicRHI()->RecoredTexture(szFullFile, TexRHI);
			}

			const char* szDuring = nodeItem.getAttribute("duration");
			long during = atol(szDuring);

			drawable->appandTex(during, TexRHI);

			nodeItem = nodeDrawable.getChildNode("item", ++j);
		}
		return drawable;
	}
	return nullptr;
}