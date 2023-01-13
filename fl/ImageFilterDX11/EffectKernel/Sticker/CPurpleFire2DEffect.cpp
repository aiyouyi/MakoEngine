#include "CPurpleFire2DEffect.h"
#include "Toolbox/Render/VertexBuffer.h"
#include "Toolbox/Render/IndexBuffer.h"
#include "Toolbox/Render/DynamicRHI.h"
#include "Toolbox/Render/ShaderRHI.h"
#include "Toolbox/Render/TextureRHI.h"
#include "Toolbox/Render/RenderTargetRHI.h"
#include <string>
#include "Toolbox/DXUtils/DX11Resource.h"
#include "RenderState/PiplelineState.h"
#include "Toolbox/Render/MaterialTexRHI.h"
#include <chrono>

CPurpleFire2DEffect::CPurpleFire2DEffect()
{
	m_EffectPart = BODY_TRACK2D_EFFECT;
}

CPurpleFire2DEffect::~CPurpleFire2DEffect()
{

}

bool CPurpleFire2DEffect::ReadConfig(XMLNode& childNode, HZIP hZip /*= 0*/, char* pFilePath /*= NULL*/)
{
	CEffectPart::ReadConfig(childNode, hZip, pFilePath);
	ReadConfig(childNode, hZip, pFilePath, "");
	return true;
}

bool CPurpleFire2DEffect::ReadConfig(XMLNode& childNode, const std::string& path)
{
	CEffectPart::ReadConfig(childNode, path);
	ReadConfig(childNode, 0, NULL, path);
	return true;
}

bool CPurpleFire2DEffect::ReadConfig(XMLNode& childNode, HZIP hZip, char* pFilePath, const std::string& path)
{
	if (!childNode.isEmpty())
	{
		XMLNode nodeDrawable = childNode.getChildNode("drawable", 0);
		if (!nodeDrawable.isEmpty())
		{
			const char* szDrawableName = nodeDrawable.getAttribute("Material");
			if (szDrawableName != NULL)
			{
				if (hZip == 0)
				{
					//AnimInfo anim_info;
					//anim_info.fps = 0;
					//anim_info.dir = path;
					//anim_info.relative_filename_list.push_back(szDrawableName);
					//m_anim_id = ResourceManager::Instance().loadAnim(anim_info);
					std::string noisePath = path + "/" + std::string(szDrawableName);
					noiseTex = GetDynamicRHI()->CreateAsynTextureFromFile(noisePath);
				}
				else
				{
					//AnimInfo anim_info;
					//anim_info.fps = 0;
					//anim_info.dir = path;
					//anim_info.relative_filename_list.push_back(szDrawableName);
					//m_anim_id = ResourceManager::Instance().loadAnimFromZip(anim_info, hZip);
					noiseTex = GetDynamicRHI()->CreateAsynTextureZIP(hZip, szDrawableName);
				}

				const char* szBlendType = nodeDrawable.getAttribute("blendType");
				m_BlendType = GetBlendType(szBlendType);

				const char* szAlpha = nodeDrawable.getAttribute("threshold");
				if (szAlpha != NULL)
				{
					m_ValueThres = atof(szAlpha);
				}

				const char* szLight = nodeDrawable.getAttribute("lightFireWeight");
				if (szLight != NULL)
				{
					fireParam.z = atof(szLight);
				}

				const char* szDark = nodeDrawable.getAttribute("darkFireWeight");
				if (szDark != NULL)
				{
					fireParam.w = atof(szDark);
				}

				const char* szLight1 = nodeDrawable.getAttribute("lightBlend");
				if (szLight1 != NULL)
				{
					clipParam.z = atof(szLight1);
				}

				const char* szDark1 = nodeDrawable.getAttribute("darkBlend");
				if (szDark1 != NULL)
				{
					clipParam.w = atof(szDark1);
				}
			}

		}
	}
	return true;
}

bool CPurpleFire2DEffect::WriteConfig(std::string& tempPath, XMLNode& root, HZIP dst, HZIP src)
{
	return true;
}

void CPurpleFire2DEffect::InitRenderTarget()
{
	if (lastInputImageRT)
	{
		lastInputImageRT.reset();
	}
	if (lastFireImageRT)
	{
		lastFireImageRT.reset();
	}
	if (lastFireImageRTHalfRes)
	{
		lastFireImageRTHalfRes.reset();
	}
	if (velocityRT)
	{
		velocityRT.reset();
	}
	if (velocityRTHalfRes)
	{
		velocityRTHalfRes.reset();
	}
	lastInputImageRT = GetDynamicRHI()->CreateRenderTarget(mWidth, mHeight, false);
	lastFireImageRT = GetDynamicRHI()->CreateRenderTarget(mWidth, mHeight, false);
	lastFireImageRTHalfRes = GetDynamicRHI()->CreateRenderTarget(mWidth * 0.5, mHeight * 0.5, false);
	velocityRT = GetDynamicRHI()->CreateRenderTarget(mWidth, mHeight, false);
	velocityRTHalfRes = GetDynamicRHI()->CreateRenderTarget(mWidth * 0.5, mHeight * 0.5, false);
}

bool CPurpleFire2DEffect::Prepare()
{
	//vb ib
	Vector3 arrCoords[4] = { Vector3(-1, -1, 1), Vector3(1, -1, 1), Vector3(-1, 1, 1), Vector3(1, 1, 1) };
	unsigned short index[] =
	{
		0, 1, 2,
		1, 2, 3
	};
	mVertexBuffer = GetDynamicRHI()->CreateVertexBuffer((float*)arrCoords, 4, 3);
	mIndexBuffer = GetDynamicRHI()->CreateIndexBuffer(index, 2);

	//shader
	velocityPassShader = GetDynamicRHI()->CreateShaderRHI();
	firePassShader = GetDynamicRHI()->CreateShaderRHI();
	blendPassShader = GetDynamicRHI()->CreateShaderRHI();
	normalPassShader = GetDynamicRHI()->CreateShaderRHI();

	if (GetDynamicRHI()->API == CC3DDynamicRHI::DX11)
	{
		CCVetexAttribute pAttribute[] =
		{
			{VERTEX_ATTRIB_POSITION, FLOAT_C3},
		};

		std::string path = m_resourcePath + "/Shader/velocityShader.fx";
		velocityPassShader->InitShader(path, pAttribute, 1);

		path = m_resourcePath + "/Shader/fireShader.fx";
		firePassShader->InitShader(path, pAttribute, 1);

		path = m_resourcePath + "/Shader/blendShader.fx";
		blendPassShader->InitShader(path, pAttribute, 1);

		path = m_resourcePath + "/Shader/normalShader.fx";
		normalPassShader->InitShader(path, pAttribute, 1);
	}
	else
	{
		
	}

	//RenderTarget   Texture2D
	//noiseTex = GetDynamicRHI()->CreateTextureFromFile( "E:/fireNoise.jpg", false);
	return true;
}
bool bInit = false;
void CPurpleFire2DEffect::Render(BaseRenderParam& RenderParam)
{
	long runTime = GetRunTime();
	if (runTime < 0)
	{
		return;
	}
	int width = RenderParam.GetWidth();
	int height = RenderParam.GetHeight();
	if (width != mWidth || height != mHeight)
	{
		mWidth = width;
		mHeight = height;
		InitRenderTarget();
	}

	std::chrono::system_clock::time_point time_point_now = std::chrono::system_clock::now();
	std::chrono::system_clock::duration duration_since_epoch = time_point_now.time_since_epoch();
	time_t microseconds_since_epoch = std::chrono::duration_cast<std::chrono::microseconds>(duration_since_epoch).count();
	//std::cout << microseconds_since_epoch % 100000000  <<std::endl;
	bool bMirror;
	int SplitScreenNum = RenderParam.GetSplitScreenNum(bMirror);

	fireParam.x = mWidth / (float)SplitScreenNum / (float)mHeight;
	double currentSeconds = double(microseconds_since_epoch);
	fireParam.y = fmod( currentSeconds * 0.000001, 1.0 ); //这个值只会影响火焰的抖动是否剧烈，基本不需要怎么调整
	//std::cout << fireParam.y <<std::endl;

	auto pDoubleBuffer = RenderParam.GetDoubleBuffer();
	std::shared_ptr<CC3DTextureRHI> bgMask = RenderParam.GetBodyMaskTexture();
	if (bgMask == nullptr)
	{
		return;
	}

	//D3D11_BLEND_DESC blend_desc;
	//memset(&blend_desc, 0, sizeof(blend_desc));
	//blend_desc.AlphaToCoverageEnable = false;
	//blend_desc.IndependentBlendEnable = false;
	//blend_desc.RenderTarget[0].BlendEnable = true;
	//blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	//blend_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	//blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
	//blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
	//blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	//blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	//blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	//ID3D11BlendState* pBlendState = NULL;
	//DevicePtr->CreateBlendState(&blend_desc, &pBlendState);

	//float blendFactor[] = { 0.f,0.f,0.f,0.f };
	//DeviceContextPtr->OMSetBlendState(pBlendState, blendFactor, 0xffffffff);
	//pBlendState->Release();

	if (FrameIndex == 0)
	{
		//保存上一帧图像
		lastInputImageRT->Bind();
		lastInputImageRT->Clear(0, 0, 0, 1);
		normalPassShader->UseShader();
		GetDynamicRHI()->SetPSShaderResource(0, pDoubleBuffer->GetFBOTextureA());
		GetDynamicRHI()->DrawPrimitive(mVertexBuffer, mIndexBuffer);
		lastInputImageRT->UnBind();
		FrameIndex++;
		return;
	}

	FrameIndex++;

	//绘制速度图
	velocityRT->Bind();
	velocityRT->Clear(0,0,0,1);
	velocityPassShader->UseShader();
	GetDynamicRHI()->SetSamplerState(CC3DPiplelineState::ClampLinerSampler);
	GetDynamicRHI()->SetPSShaderResource( 0,pDoubleBuffer->GetFBOTextureA());
	GetDynamicRHI()->SetPSShaderResource( 1, lastInputImageRT );
	GetDynamicRHI()->SetPSShaderResource( 2, bgMask);
	GetDynamicRHI()->DrawPrimitive(mVertexBuffer, mIndexBuffer);
	velocityRT->UnBind();

	//速度图降低分辨率
	velocityRTHalfRes->Bind();
	velocityRTHalfRes->Clear(0,0,0,1);
	normalPassShader->UseShader();
	GetDynamicRHI()->SetPSShaderResource(0, velocityRT);
	GetDynamicRHI()->DrawPrimitive(mVertexBuffer, mIndexBuffer);
	velocityRTHalfRes->UnBind();

	//绘制结果火焰图
	lastFireImageRT->Bind();
	lastFireImageRT->Clear(0,0,0,1);
	firePassShader->UseShader();

	Vector4 thresHold;
	thresHold.x = m_ValueThres;
	GET_SHADER_STRUCT_MEMBER(FirePassConstBuffer).SetParameter("param", fireParam);
	GET_SHADER_STRUCT_MEMBER(FirePassConstBuffer).SetParameter("thres", thresHold);
	GET_SHADER_STRUCT_MEMBER(FirePassConstBuffer).ApplyToAllBuffer();
	if (noiseTex)
	{
		noiseTex->Bind(0);
	}
	GetDynamicRHI()->SetSamplerState(CC3DPiplelineState::MirrorLinerSampler, 1);
	GetDynamicRHI()->SetPSShaderResource( 1, velocityRTHalfRes );
	GetDynamicRHI()->SetPSShaderResource( 2, lastFireImageRTHalfRes );
	GetDynamicRHI()->SetPSShaderResource( 3, pDoubleBuffer->GetFBOTextureA());
	GetDynamicRHI()->DrawPrimitive(mVertexBuffer, mIndexBuffer);
	lastFireImageRT->UnBind();


	//火焰图降低分辨率
	lastFireImageRTHalfRes->Bind();
	lastFireImageRTHalfRes->Clear(0, 0, 0, 1);
	normalPassShader->UseShader();
	GetDynamicRHI()->SetPSShaderResource(0, lastFireImageRT);
	GetDynamicRHI()->DrawPrimitive(mVertexBuffer, mIndexBuffer);
	lastFireImageRTHalfRes->UnBind();

	float blendFactor[] = { 0.f,0.f,0.f,0.f };
	GetDynamicRHI()->SetBlendState(CC3DPiplelineState::BlendAlphaOn, blendFactor, 0xffffffff);
	//融合最终效果
	pDoubleBuffer->SyncAToB();
	pDoubleBuffer->BindFBOA();
	RHIResourceCast(pDoubleBuffer.get())->GetFBOA()->clear(0, 0, 0, 0);
	for (int i = 0; i < SplitScreenNum; i++ )
	{
		clipParam.x = SplitScreenNum;
		clipParam.y = i;
	
		pDoubleBuffer->BindFBOA();
		blendPassShader->UseShader();

		GET_SHADER_STRUCT_MEMBER(BlendPassConstBuffer).SetParameter("clip", clipParam);
		GET_SHADER_STRUCT_MEMBER(BlendPassConstBuffer).SetParameter("thres1", thresHold);
		GET_SHADER_STRUCT_MEMBER(BlendPassConstBuffer).ApplyToAllBuffer();

		GetDynamicRHI()->SetPSShaderResource(0, pDoubleBuffer->GetFBOTextureB());
		GetDynamicRHI()->SetPSShaderResource(1, lastFireImageRT);
		GetDynamicRHI()->SetPSShaderResource(2, bgMask);
		GetDynamicRHI()->DrawPrimitive(mVertexBuffer, mIndexBuffer);

		if (i == 0)
		{
			//保存上一帧输入图像,这个流程应该要放到最后
			lastInputImageRT->Bind();
			lastInputImageRT->Clear(0, 0, 0, 1);
			normalPassShader->UseShader();
			GetDynamicRHI()->SetPSShaderResource(0, pDoubleBuffer->GetFBOTextureA());
			GetDynamicRHI()->DrawPrimitive(mVertexBuffer, mIndexBuffer);
			lastInputImageRT->UnBind();
		}
	}

	pDoubleBuffer->unBindFBOA();


}

void CPurpleFire2DEffect::Release()
{
	
}

