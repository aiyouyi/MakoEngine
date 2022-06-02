#include "CDeNoiseFilterEffect.h"
#include <math.h>
#include <string.h>
#include "Toolbox/Helper.h"
#include "EffectKernel/ShaderProgramManager.h"

CDeNoiseFilterEffect::CDeNoiseFilterEffect() {
	m_bInit = false;

	m_pConstantBuffer = nullptr;

	m_rectVerticeBuffer = nullptr;
	m_rectIndexBuffer = nullptr;

	m_denoiseProgram = nullptr;
	m_greyProgram = nullptr;
	m_kgAndResCovProgram = nullptr;


	m_lastFilterRes = nullptr;
	memset(m_recordImageArray, 0, const_record_fbo_num * sizeof(DX11FBO *));
	m_lastFilterCovTexture = nullptr;


	m_vMeasureCov = 50.0 / 255.0;
	m_vPredictionCov = m_vMeasureCov / 1.5;
	m_vFilterCov = 0;
	m_fWeight = 1.0f;

	m_bInitRecordImage = false;
	m_lastIndex = 0;

	m_EffectPart = DENOISE_EFFECT;
}

CDeNoiseFilterEffect::~CDeNoiseFilterEffect() {
	Release();
}

void CDeNoiseFilterEffect::Release() {
	m_bInit = false;

	SAFE_UNLOCK(m_lastFilterRes);
	for (int i = 0; i < const_record_fbo_num; ++i)
		SAFE_UNLOCK(m_recordImageArray[i]);
	m_bInitRecordImage = false;

	SAFE_UNLOCK(m_lastFilterCovTexture);
}

void* CDeNoiseFilterEffect::Clone() {
	auto tmp = new CDeNoiseFilterEffect();
	tmp->Prepare();
	return tmp;
}

/* xml format
<typeeffect type = "DeNoise">
</typeeffect>
or
<typeeffect type = "DeNoise"/>
*/
bool CDeNoiseFilterEffect::ReadConfig(XMLNode& childNode, HZIP hZip, char *pFilePath) {
	if (!childNode.isEmpty())
		return true;
	return false;
}

bool CDeNoiseFilterEffect::Prepare() {
	if (!m_bInit)
	{
		m_bInit = true;

		float x = 0;
		float y = 0;
		float height = 1.0f;
		float width = 1.0f;
		XMFLOAT3 arrCoords[] = { XMFLOAT3(x,y,0), XMFLOAT3(x,y + height,0), XMFLOAT3(x + width, y + height,0), XMFLOAT3(x + width, y,0) };
		for (int i = 0; i < 4; ++i)
		{
			arrCoords[i].x = arrCoords[i].x*2.0f - 1.0f;
			arrCoords[i].y = -(arrCoords[i].y*2.0f - 1.0f);
		}


		//创建顶点buffer
		BaseRectVertex vertices[] =
		{
			{ arrCoords[0], XMFLOAT2(0.0f, 0.0f) },
			{ arrCoords[1], XMFLOAT2(0.0f, 1.0f) },
			{ arrCoords[2], XMFLOAT2(1.0f, 1.0f) },
			{ arrCoords[3], XMFLOAT2(1.0f, 0.0f) }
		};
		D3D11_BUFFER_DESC verBufferDesc;
		memset(&verBufferDesc, 0, sizeof(D3D11_BUFFER_DESC));
		verBufferDesc.ByteWidth = sizeof(BaseRectVertex) * 4;
		verBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		verBufferDesc.CPUAccessFlags = 0;
		verBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		D3D11_SUBRESOURCE_DATA vertexInitData;
		memset(&vertexInitData, 0, sizeof(D3D11_SUBRESOURCE_DATA));
		vertexInitData.pSysMem = vertices;
		DevicePtr->CreateBuffer(&verBufferDesc, &vertexInitData, &m_rectVerticeBuffer);


		//创建索引buffer
		WORD index[] =
		{
			0, 1, 2,
			0, 2, 3
		};
		D3D11_BUFFER_DESC indexBufferDesc;
		memset(&indexBufferDesc, 0, sizeof(D3D11_BUFFER_DESC));
		indexBufferDesc.ByteWidth = sizeof(WORD) * 6;
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags = 0;
		indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		D3D11_SUBRESOURCE_DATA indexInitData;
		memset(&indexInitData, 0, sizeof(D3D11_SUBRESOURCE_DATA));
		indexInitData.pSysMem = index;
		DevicePtr->CreateBuffer(&indexBufferDesc, &indexInitData, &m_rectIndexBuffer);



		//创建shader
		string path = m_resourcePath + "/Shader/Denoise.fx";
		m_denoiseProgram = ShaderProgramManager::GetInstance()->GetOrCreateShaderByPathAndAttribs(path, nullptr, 0);

		string path1 = m_resourcePath + "/Shader/BGR2RGB.fx";
		m_greyProgram = ShaderProgramManager::GetInstance()->GetOrCreateShaderByPathAndAttribs(path1, nullptr, 0);

		string path2 = m_resourcePath + "/Shader/KGAndCov.fx";
		m_kgAndResCovProgram = ShaderProgramManager::GetInstance()->GetOrCreateShaderByPathAndAttribs(path2, nullptr, 0);

		//创建constbuffer 参数
		D3D11_BUFFER_DESC bd;
		memset(&bd, 0, sizeof(bd));
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(RectConstantBuffer);
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bd.CPUAccessFlags = 0;
		auto hr = DevicePtr->CreateBuffer(&bd, NULL, &m_pConstantBuffer);
	}

	return true;
}

void CDeNoiseFilterEffect::GreyRender(BaseRenderParam &RenderParam) {
	auto pDoubleBuffer = RenderParam.GetDoubleBuffer();
	unsigned int nStride = sizeof(BaseRectVertex);
	unsigned int nOffset = 0;
	int greyWidth = RenderParam.GetWidth() / 3;
	int greyHeight = RenderParam.GetHeight() / 3;

	DX11FBO *fbSmallGrey = FBOCacheInst->fetchFrameBufferForSize(greyWidth, greyHeight);
	{
		fbSmallGrey->bind();

		//设置shader
		m_greyProgram->useShader();

		//设置矩阵变换
		RectConstantBuffer mWVP;
		mWVP.mWVP = XMMatrixIdentity();
		DeviceContextPtr->UpdateSubresource(m_pConstantBuffer, 0, NULL, &mWVP, 0, 0);
		DeviceContextPtr->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);

		//设置纹理以及纹理采样
		ID3D11ShaderResourceView *pMyShaderResourceView = pDoubleBuffer->GetFBOTextureA()->getTexShaderView();
		DeviceContextPtr->PSSetShaderResources(0, 1, &pMyShaderResourceView);
		DeviceContextPtr->PSSetSamplers(0, 1, &m_pSamplerLinear);

		//设置顶点数据
		DeviceContextPtr->IASetVertexBuffers(0, 1, &m_rectVerticeBuffer, &nStride, &nOffset);
		DeviceContextPtr->IASetIndexBuffer(m_rectIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

		DeviceContextPtr->DrawIndexed(6, 0, 0);
	}

	if (m_lastFilterCovTexture == NULL)
	{
		m_lastFilterCovTexture = FBOCacheInst->fetchFrameBufferForSize(greyWidth, greyHeight);
		m_lastFilterCovTexture->bind();
		float ClearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f }; // rgba  
		DeviceContextPtr->ClearRenderTargetView(m_lastFilterCovTexture->getRenderTargetView(), ClearColor);
	}

	if (!m_bInitRecordImage)
	{
		m_bInitRecordImage = true;
		for (int i = 0; i < const_record_fbo_num; ++i)
		{
			m_recordImageArray[i] = fbSmallGrey;
			fbSmallGrey->lock();
		}
	}
	else
	{
		m_recordImageArray[m_lastIndex]->unlock();
		m_recordImageArray[m_lastIndex] = fbSmallGrey;

		m_lastIndex = (m_lastIndex + 1) % const_record_fbo_num;

		fbSmallGrey->lock();
	}

	fbSmallGrey->unlock();
	fbSmallGrey = NULL;
}

void CDeNoiseFilterEffect::kgAndResCovRender(BaseRenderParam &RenderParam) {
	unsigned int nStride = sizeof(BaseRectVertex);
	unsigned int nOffset = 0;
	int greyWidth = RenderParam.GetWidth() / 3;
	int greyHeight = RenderParam.GetHeight() / 3;

	DX11FBO *fbFilterCov = FBOCacheInst->fetchFrameBufferForSize(greyWidth, greyHeight);
	{
		m_kgAndResCovProgram->useShader();
		fbFilterCov->bind();

		float ClearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f }; // rgba  
		DeviceContextPtr->ClearRenderTargetView(fbFilterCov->getRenderTargetView(), ClearColor);

		//设置矩阵变换
		RectConstantBuffer mWVP;
		mWVP.mWVP = XMMatrixIdentity();
		DeviceContextPtr->UpdateSubresource(m_pConstantBuffer, 0, NULL, &mWVP, 0, 0);
		DeviceContextPtr->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);

		//设置纹理以及纹理采样
		for (int i = 0; i < 6; ++i)
		{
			ID3D11ShaderResourceView *pMyShaderResourceView = m_recordImageArray[(i + m_lastIndex) % const_record_fbo_num]->getTexture()->getTexShaderView();
			DeviceContextPtr->PSSetShaderResources(i, 1, &pMyShaderResourceView);
		}
		ID3D11ShaderResourceView *pMyShaderResourceView = m_lastFilterCovTexture->getTexture()->getTexShaderView();
		DeviceContextPtr->PSSetShaderResources(6, 1, &pMyShaderResourceView);
		DeviceContextPtr->PSSetSamplers(0, 1, &m_pSamplerLinear);

		//设置顶点数据
		DeviceContextPtr->IASetVertexBuffers(0, 1, &m_rectVerticeBuffer, &nStride, &nOffset);
		DeviceContextPtr->IASetIndexBuffer(m_rectIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

		DeviceContextPtr->DrawIndexed(6, 0, 0);
	}
	m_lastFilterCovTexture->unlock();
	m_lastFilterCovTexture = fbFilterCov;
}

void CDeNoiseFilterEffect::DenoiseRender(BaseRenderParam &RenderParam) {
	auto pDoubleBuffer = RenderParam.GetDoubleBuffer();
	unsigned int nStride = sizeof(BaseRectVertex);
	unsigned int nOffset = 0;
	int greyWidth = RenderParam.GetWidth() / 3;
	int greyHeight = RenderParam.GetHeight() / 3;

	{
		m_denoiseProgram->useShader();
		pDoubleBuffer->BindFBOB();

		float ClearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f }; // rgba  
		DeviceContextPtr->ClearRenderTargetView(pDoubleBuffer->GetFBOB()->getRenderTargetView(), ClearColor);

		//设置矩阵变换
		RectConstantBuffer mWVP;
		mWVP.mWVP = XMMatrixIdentity();
		DeviceContextPtr->UpdateSubresource(m_pConstantBuffer, 0, NULL, &mWVP, 0, 0);
		DeviceContextPtr->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);

		//设置纹理以及纹理采样
		ID3D11ShaderResourceView *pMyShaderResourceView = pDoubleBuffer->GetFBOTextureA()->getTexShaderView();;
		DeviceContextPtr->PSSetShaderResources(0, 1, &pMyShaderResourceView);
		pMyShaderResourceView = pDoubleBuffer->GetFBOTextureA()->getTexShaderView();;
		if (m_lastFilterRes != NULL)
		{
			pMyShaderResourceView = m_lastFilterRes->getTexture()->getTexShaderView();
		}
		DeviceContextPtr->PSSetShaderResources(1, 1, &(pMyShaderResourceView));
		pMyShaderResourceView = m_lastFilterCovTexture->getTexture()->getTexShaderView();

		//pMyShaderResourceView = m_recordImageArray[1]->getTexture()->getTexShaderView();

		DeviceContextPtr->PSSetShaderResources(2, 1, &(pMyShaderResourceView));
		DeviceContextPtr->PSSetSamplers(0, 1, &m_pSamplerLinear);

		//设置顶点数据
		DeviceContextPtr->IASetVertexBuffers(0, 1, &m_rectVerticeBuffer, &nStride, &nOffset);
		DeviceContextPtr->IASetIndexBuffer(m_rectIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

		DeviceContextPtr->DrawIndexed(6, 0, 0);
	}

	if (m_lastFilterRes != NULL)
	{
		m_lastFilterRes->unlock();
	}

	DX11FBO *fbFilterRes = FBOCacheInst->fetchFrameBufferForSize(RenderParam.GetWidth(), RenderParam.GetHeight());
	unsigned int sub = D3D11CalcSubresource(0, 0, 1);
	DeviceContextPtr->ResolveSubresource(
		fbFilterRes->getTexture()->getTex(),
		sub,
		pDoubleBuffer->GetFBOTextureB()->getTex(),
		sub,
		DXGI_FORMAT_R8G8B8A8_UNORM
	);
	
	m_lastFilterRes = fbFilterRes;
}

void CDeNoiseFilterEffect::Render(BaseRenderParam &RenderParam) {

	if (m_alpha < 0.001f)
	{
		return;
	}

	Prepare();


	auto pDoubleBuffer = RenderParam.GetDoubleBuffer();
	pDoubleBuffer->SyncAToB();

	//1.绘制灰度图
	GreyRender(RenderParam);
	//2. 生成滤波控制纹理
	kgAndResCovRender(RenderParam);
	//3. 滤波计算
	DenoiseRender(RenderParam);
	
	//DeviceContextPtr->CopyResource(pDoubleBuffer->GetFBOTextureB()->getTex(), m_lastFilterRes->getTexture()->getTex());

	pDoubleBuffer->SwapFBO();
}

void CDeNoiseFilterEffect::reset() {
	SAFE_UNLOCK(m_lastFilterRes);
	for (int i = 0; i < const_record_fbo_num; ++i)
		SAFE_UNLOCK(m_recordImageArray[i]);
	m_bInitRecordImage = false;

	SAFE_UNLOCK(m_lastFilterCovTexture);
}