#include "Toolbox/DXUtils/DX11Texture.h"
#include "Toolbox/DXUtils/DX11Shader.h"
#include "Toolbox/DXUtils/DX11Context.h"
#include "Toolbox/Render/DynamicRHI.h"
#include "Toolbox/Render/TextureRHI.h"
#include "Toolbox/Render/VertexBuffer.h"
#include "Toolbox/Render/IndexBuffer.h"
#include "ShaderProgramManager.h"
#include "VideoInfo.h"
#include "ResourceManager.h"
#include "Toolbox/RenderState/PiplelineState.h"
#include "Toolbox/Render/RenderTargetRHI.h"
#include "Toolbox/DXUtils/DX11Resource.h"
#include "VideoAnimation.h"

VideoAnimation::VideoAnimation()
{

}

VideoAnimation::~VideoAnimation()
{

}

void VideoAnimation::Init(const std::string& m_resourcePath, int videoWidth, int videoHeight)
{
	//renderMaskFbo = std::make_shared<DX11FBO>();
	//renderMaskFbo->initWithTexture(videoWidth, videoHeight, false, nullptr, DXGI_FORMAT_B8G8R8A8_UNORM, false); //视频解析出来是BGRA格式
	//renderResultFbo = std::make_shared<DX11FBO>();
	//renderResultFbo->initWithTexture(videoWidth, videoHeight, false, nullptr);
	renderMaskFbo = GetDynamicRHI()->CreateRenderTarget( videoWidth, videoHeight, false, nullptr, CC3DTextureRHI::SFT_A8B8G8R8); //  1 == DXGI_FORMAT_B8G8R8A8_UNORM
	renderResultFbo = GetDynamicRHI()->CreateRenderTarget(videoWidth, videoHeight, false);
	maskTex = GetDynamicRHI()->CreateTexture(CC3DTextureRHI::SFT_A8B8G8R8, CC3DTextureRHI::OT_NONE, videoWidth, videoHeight, nullptr, 4 * videoWidth);

	Vector3 arrCoords[4] = { Vector3(-1, -1, 1), Vector3(1, -1, 1), Vector3(-1, 1, 1), Vector3(1, 1, 1) };
	unsigned short index[] =
	{
		0, 1, 2,
		2, 1, 3
	};
	mVertexBuffer = GetDynamicRHI()->CreateVertexBuffer((float*)arrCoords, 4, 3);
	mIndexBuffer = GetDynamicRHI()->CreateIndexBuffer(index, 2);

	CCVetexAttribute pAttribute2[] =
	{
		{VERTEX_ATTRIB_POSITION, FLOAT_C3},
	};

	if (pShaderRect == nullptr)
	{
		std::string fsPath = m_resourcePath + "/Shader/3D/MaskBlend.fx";
		pShaderRect = ShaderProgramManager::GetInstance()->GetOrCreateShaderByPathAndAttribs(fsPath, pAttribute2, 1);
	}

	CCVetexAttribute pAttribute3[] =
	{
		{VERTEX_ATTRIB_POSITION, FLOAT_C2},
		{VERTEX_ATTRIB_TEX_COORD, FLOAT_C2}
	};
	if (pShaderBlend == nullptr)
	{
		std::string fsPath = m_resourcePath + "/Shader/3D/ProfileBlend.fx";
		pShaderBlend = ShaderProgramManager::GetInstance()->GetOrCreateShaderByPathAndAttribs(fsPath, pAttribute3, 2, true);
	}
}

bool VideoAnimation::ReadJson(const nlohmann::json& root)
{
	nlohmann::json frameJson = root["FrameRect"];
	if (!frameJson.is_null())
	{
		for (int ni = 0; ni < frameJson.size(); ni++)
		{
			nlohmann::json frameRect = frameJson[ni];
			int frameIndex = frameRect["Frame"].get<int>();

			nlohmann::json maskJson = frameRect["Mask"];
			maxMaskCount = (std::max)(maxMaskCount, (int)maskJson.size());
			std::map<int, std::vector<float>> maskRectMap;
			for (int nj = 0; nj < maskJson.size(); nj++)
			{
				nlohmann::json indexRect = maskJson[nj];
				int index = indexRect["Index"].get<int>();
				std::vector<float> rect(8);
				rect[0] = indexRect["Rect"][0].get<float>();
				rect[1] = indexRect["Rect"][1].get<float>();
				rect[2] = indexRect["Rect"][2].get<float>();
				rect[3] = indexRect["Rect"][3].get<float>();
				rect[4] = indexRect["Rect"][4].get<float>();
				rect[5] = indexRect["Rect"][5].get<float>();
				rect[6] = indexRect["Rect"][6].get<float>();
				rect[7] = indexRect["Rect"][7].get<float>();
				maskRectMap[index] = rect;			
			}

			frameMaskMap[frameIndex] = maskRectMap;
		}
	}

	return true;
}

bool VideoAnimation::ReadJsonFromZip(const std::string& jsonName, HZIP hZip){
	
	if (hZip != NULL)
	{
		ZIPENTRY ze;
		int index;
		if (ZR_OK == FindZipItem(hZip, jsonName.c_str(), true, &index, &ze))
		{
			char* pDataBuffer = new char[ze.unc_size];
			ZRESULT res = UnzipItem(hZip, index, pDataBuffer, ze.unc_size);
			if (res != ZR_OK)
			{
				delete[]pDataBuffer;
				CloseZip(hZip);
				return false;
			}
			nlohmann::json root = nlohmann::json::parse(pDataBuffer, pDataBuffer + ze.unc_size);
			delete[]pDataBuffer;

			nlohmann::json frameJson = root["FrameRect"];
			if (!frameJson.is_null())
			{
				for (int ni = 0; ni < frameJson.size(); ni++)
				{
					nlohmann::json frameRect = frameJson[ni];
					int frameIndex = frameRect["Frame"].get<int>();

					nlohmann::json maskJson = frameRect["Mask"];
					maxMaskCount = (std::max)(maxMaskCount, (int)maskJson.size());
					std::map<int, std::vector<float>> maskRectMap;
					for (int nj = 0; nj < maskJson.size(); nj++)
					{
						nlohmann::json indexRect = maskJson[nj];
						int index = indexRect["Index"].get<int>();
						std::vector<float> rect(8);
						rect[0] = indexRect["Rect"][0].get<float>();
						rect[1] = indexRect["Rect"][1].get<float>();
						rect[2] = indexRect["Rect"][2].get<float>();
						rect[3] = indexRect["Rect"][3].get<float>();
						rect[4] = indexRect["Rect"][4].get<float>();
						rect[5] = indexRect["Rect"][5].get<float>();
						rect[6] = indexRect["Rect"][6].get<float>();
						rect[7] = indexRect["Rect"][7].get<float>();
						maskRectMap[index] = rect;
					}

					frameMaskMap[frameIndex] = maskRectMap;
				}
				return true;
			}
			else
			{
				CloseZip(hZip);
				return false;
			}



		}
		return false;
	}
	return false;

}

bool VideoAnimation::ReadJsonFromFile(const std::string& jsonName) {


	std::ifstream inStream(jsonName, ios_base::in | ios_base::binary);
	if (!inStream.is_open())
	{
		return false;
	}
	std::stringstream buffer;
	buffer << inStream.rdbuf();
	std::string contents(buffer.str());
	inStream.close();

	char* pDataBuffer = (char*)contents.data();

	nlohmann::json root = nlohmann::json::parse(pDataBuffer, pDataBuffer + contents.length());

	nlohmann::json frameJson = root["FrameRect"];
	if (!frameJson.is_null())
	{
		for (int ni = 0; ni < frameJson.size(); ni++)
		{
			nlohmann::json frameRect = frameJson[ni];
			int frameIndex = frameRect["Frame"].get<int>();

			nlohmann::json maskJson = frameRect["Mask"];
			maxMaskCount = (std::max)(maxMaskCount, (int)maskJson.size());
			std::map<int, std::vector<float>> maskRectMap;
			for (int nj = 0; nj < maskJson.size(); nj++)
			{
				nlohmann::json indexRect = maskJson[nj];
				int index = indexRect["Index"].get<int>();
				std::vector<float> rect(8);
				rect[0] = indexRect["Rect"][0].get<float>();
				rect[1] = indexRect["Rect"][1].get<float>();
				rect[2] = indexRect["Rect"][2].get<float>();
				rect[3] = indexRect["Rect"][3].get<float>();
				rect[4] = indexRect["Rect"][4].get<float>();
				rect[5] = indexRect["Rect"][5].get<float>();
				rect[6] = indexRect["Rect"][6].get<float>();
				rect[7] = indexRect["Rect"][7].get<float>();
				maskRectMap[index] = rect;
			}

			frameMaskMap[frameIndex] = maskRectMap;
		}
		return true;
	}


	return false;
	
}

void VideoAnimation::SetMaskWriteAlpha(int index,bool alphaOn)
{
	if (index >= maxMaskCount)
	{
		return;
	}
	indexWriteAlpha[index] = alphaOn;
}

void VideoAnimation::SetTexture(int index, long long id)
{
	if (index >= maxMaskCount)
	{
		return;
	}
	//如果要替换图片TODO

	auto itr = indexMaskTextureMap.find(index);
	if (itr != indexMaskTextureMap.end())
	{
		itr->second.reset();
	}
	Image* img = ResourceManager::Instance().getImage(id);
	indexMaskTextureMap[index] = img->tex;
}

void VideoAnimation::SetTexture(int index, const std::string& texturePath)
{
	if (index >= maxMaskCount)
	{
		return;
	}
	//如果要替换图片TODO

	auto itr = indexMaskTextureMap.find(index);
	if (itr != indexMaskTextureMap.end() )
	{
		itr->second.reset();
	}

	std::shared_ptr<CC3DTextureRHI> avatar = GetDynamicRHI()->CreateTextureFromFile(texturePath, false);
	indexMaskTextureMap[index] = avatar;
}

std::shared_ptr<CC3DRenderTargetRHI> VideoAnimation::Render(Video* vdo)
{
	int currentFrame = vdo->cur_frame_idx;
	auto itr = frameMaskMap.find(currentFrame);
	if (itr != frameMaskMap.end())
	{

		renderMaskFbo->Bind();
		renderMaskFbo->Clear(0, 0, 0, 0);
		pShaderBlend->useShader();
		GetDynamicRHI()->SetPSShaderResource(1, maskTex);

		float blendFactor[] = { 0.f,0.f,0.f,0.f };
		std::map<int, std::vector<float>> IndexMaskMap = itr->second;
		for (auto maskItr = IndexMaskMap.begin(); maskItr != IndexMaskMap.end(); maskItr++)
		{
			std::vector<float>& vert = maskItr->second;
			int maskIndex = maskItr->first;
			
			GetDynamicRHI()->SetBlendState(CC3DPiplelineState::BlendDisable, blendFactor, 0xffffffff);

			if (GetDynamicRHI()->API == CC3DDynamicRHI::DX11)
			{
				if (mRectVertexBuffer[maskIndex][0] == nullptr)
				{
					mRectVertexBuffer[maskIndex][0] = GetDynamicRHI()->CreateVertexBuffer((float*)&vert[0], 4, 2);
				}
				else
				{
					mRectVertexBuffer[maskIndex][0]->UpdateVertexBUffer((float*)&vert[0], 4, 2 * sizeof(float), 2 * sizeof(float), 0);
				}

				if (mRectVertexBuffer[maskIndex][1] == nullptr)
				{
					mRectVertexBuffer[maskIndex][1] = GetDynamicRHI()->CreateVertexBuffer(&g_TextureCoordinate[0], 4, 2);
				}
			}
			else if (GetDynamicRHI()->API == CC3DDynamicRHI::OPENGL)
			{
				if ( mRectVertexBuffer[maskIndex][0] == nullptr )
				{
					mRectVertexBuffer[maskIndex][0] = GetDynamicRHI()->CreateVertexBuffer(2);
					mRectVertexBuffer[maskIndex][0]->CreateVertexBuffer( (float*)&vert[0], 4, 2, 0 );
					mRectVertexBuffer[maskIndex][0]->CreateVertexBuffer( (float*)&g_TextureCoordinate[0], 4, 2, 1 );
				}
				else
				{
					mRectVertexBuffer[maskIndex][0]->UpdateVertexBUffer( (float*)&vert[0], 4, 2 * sizeof(float), 2 * sizeof(float), 0);
				}
			}


			auto texItr = indexMaskTextureMap.find(maskIndex);
			if (texItr != indexMaskTextureMap.end())
			{
				GetDynamicRHI()->SetPSShaderResource(0, indexMaskTextureMap[maskIndex]);
				
				GetDynamicRHI()->DrawPrimitive(mRectVertexBuffer[maskIndex], 2, mIndexBuffer);
			}
		}
		renderMaskFbo->UnBind();

		//mask图贴到最终结果上
		GetDynamicRHI()->SetBlendState(CC3DPiplelineState::BlendDisable, blendFactor, 0xffffffff);
		renderResultFbo->Bind();
		renderResultFbo->Clear(0, 0, 0, 0);
		pShaderRect->useShader();
		//auto pVideoTex = RHIResourceCast(vdo->current_frame->tex.get())->GetSRV();
		//DeviceContextPtr->PSSetSamplers(0, 1, &m_pSamplerLinear);
		//DeviceContextPtr->PSSetShaderResources(0, 1, &pVideoTex);
		//auto pMaskTex = RHIResourceCast(renderMaskFbo.get())->GetTexture()->getTexShaderView();
		//DeviceContextPtr->PSSetSamplers(1, 1, &m_pSamplerLinear);
		//DeviceContextPtr->PSSetShaderResources(1, 1, &pMaskTex);
		GetDynamicRHI()->SetPSShaderResource(0, vdo->current_frame->tex);
		GetDynamicRHI()->SetPSShaderResource(1, renderMaskFbo);
		GetDynamicRHI()->DrawPrimitive(mVertexBuffer, mIndexBuffer);
		renderResultFbo->UnBind();
		//GetDynamicRHI()->SetBlendState(CC3DPiplelineState::BlendAlphaOn, blendFactor, 0xffffffff);
	}
	else
	{
		renderMaskFbo->Bind();
		renderMaskFbo->Clear(0, 0, 0, 0);

		float blendFactor[] = { 0.f,0.f,0.f,0.f };
		GetDynamicRHI()->SetBlendState(CC3DPiplelineState::BlendDisable, blendFactor, 0xffffffff);
		renderResultFbo->Bind();
		renderResultFbo->Clear(0, 0, 0, 0);
		pShaderRect->useShader();
		GetDynamicRHI()->SetPSShaderResource(0, vdo->current_frame->tex);
		GetDynamicRHI()->SetPSShaderResource(1, renderMaskFbo);
		GetDynamicRHI()->DrawPrimitive(mVertexBuffer, mIndexBuffer);
		renderResultFbo->UnBind();
		//GetDynamicRHI()->SetBlendState(CC3DPiplelineState::BlendAlphaOn, blendFactor, 0xffffffff);
	}

	float blendFactor[] = { 0.f,0.f,0.f,0.f };
	GetDynamicRHI()->SetBlendState(CC3DPiplelineState::BlendAlphaOn, blendFactor, 0xffffffff);

	return renderResultFbo;
}

std::shared_ptr<CC3DRenderTargetRHI> VideoAnimation::GetMaskFbo() noexcept
{
	return renderMaskFbo;
}

std::shared_ptr<CC3DTextureRHI> VideoAnimation::GetMaskTex()
{
	return maskTex;
}

