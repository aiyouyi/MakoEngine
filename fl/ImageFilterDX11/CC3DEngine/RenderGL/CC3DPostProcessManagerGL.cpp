#include "CC3DPostProcessManagerGL.h"
#include <BaseDefine/Vectors.h>
#include <GL/openglutil.h>


#define DUAL_SAMPLE_COUNT 5

PostProcessManagerGL::PostProcessManagerGL()
{
	m_HalfSizeBuffer = nullptr;
	m_FullSizeBuffer = nullptr;
}

PostProcessManagerGL::~PostProcessManagerGL()
{
	//for (int i = 0; i < RECTATTRIBS; i++)
	//{
	//	GL_DELETE_BUFFER(mVbo[i]);
	//}
	//GL_DELETE_VAO(mVao);
	//GL_DELETE_BUFFER(mEbo);
	SAFE_DELETE(m_HalfSizeBuffer);
	SAFE_DELETE(m_FullSizeBuffer);

	for (int ni = 0; ni < bloomLevelFBO.size(); ni++)
	{
		SAFE_DELETE(bloomLevelFBO[ni]);
	}

	bloomLevelFBO.clear();
}

void PostProcessManagerGL::Init(int nWidth, int nHeight)
{
	if (m_HalfSizeBuffer == nullptr)
	{
		Vector3 arrCoords[VERT_NUM] = { Vector3(-1, -1, 1), Vector3(3, -1, 1), Vector3(-1, 3, 1) };
		//for (int i = 0; i < 4; ++i)
		//{
		//	arrCoords[i].x = arrCoords[i].x * 2.0f - 1.0f;
		//	arrCoords[i].y = -(arrCoords[i].y * 2.0f - 1.0f);
		//}

		//Vector2 texCoord[VERT_NUM] = { Vector2(0.0, 0.0), Vector2(0.0, 1.0), Vector2(1.0f, 1.0f), Vector2(1.0f, 1.0f) };

		uint16 index[] = {
			0, 1, 2
		};

		//glGenBuffers(1, &mEbo);
		//int pArrange = 3;
		//OpenGLUtil::createVAO(mVao, mVbo[0], (float*)arrCoords, 3, &pArrange, 1, 3);

		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEbo);
		//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16) * 3, &index[0], GL_STATIC_DRAW);
		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		postProcessTriMesh = GetDynamicRHI()->CreateVertexBuffer(1);
		postProcessTriMesh->CreateVertexBuffer((float*)arrCoords, 3, 3, 0);
		postProcessIndexBuffer = GetDynamicRHI()->CreateIndexBuffer(index, 1);
		InputTextureRHI = GetDynamicRHI()->CreateTexture();

		InitFrameBuffer(nWidth, nHeight);
		
	}
	else if (_Width != nWidth || _Height != nHeight)
	{
		SAFE_DELETE(m_FullSizeBuffer);
		SAFE_DELETE(m_HalfSizeBuffer);
		InitFrameBuffer(nWidth, nHeight);
	}

	_Width = nWidth;
	_Height = nHeight;

}

void PostProcessManagerGL::InitFrameBuffer(int nWidth, int nHeight)
{
	m_FullSizeBuffer = new CCFrameBuffer();
	m_FullSizeBuffer->init(nWidth, nHeight, false, GL_RGBA16F, GL_RGBA, GL_FLOAT);

	m_HalfSizeBuffer = new CCFrameBuffer();
	m_HalfSizeBuffer->init(nWidth*m_scaleHW, nHeight*m_scaleHW, false, GL_RGBA16F, GL_RGBA, GL_FLOAT);

	for (int ni = 0; ni < DUAL_SAMPLE_COUNT; ni++)
	{
		CCFrameBuffer* fbo = new CCFrameBuffer();
		fbo->init(nWidth * m_scaleHW * pow(0.5, ni + 1), nHeight * m_scaleHW * pow(0.5, ni + 1), false, GL_RGBA16F, GL_RGBA, GL_FLOAT);
		bloomLevelFBO.emplace_back(fbo);
	}
}

uint32 PostProcessManagerGL::Process()
{
	GLuint inputTex = m_FBO_Ext->textureID;

	if (m_bloomAlpha>0.001)
	{
		_enableBloom = true;
	}
	else
	{
		_enableBloom = false;
	}

	if (_enableBloom)
	{
		inputTex = Bloom(m_FBO_Ext);
	}

	if (_enableFxaa)
	{
		m_FullSizeBuffer->bind();
		Fxaa(inputTex, m_FBO_Ext->width, m_FBO_Ext->height);
		inputTex = m_FullSizeBuffer->textureID;	
	}

	return inputTex;

}

void PostProcessManagerGL::Fxaa(uint32 input, int nWidth, int nHeight)
{
	//glViewport(0, 0, nWidth, nHeight);
	//pFxaaShader->Use();
	//glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, input);
	//pFxaaShader->SetUniform1i("colorBuffer", 0);

	//pFxaaShader->SetUniform4f("resolution", nWidth, nHeight, 1.0 / nWidth, 1.0 / nHeight);
	//pFxaaShader->SetUniform2f("clipControl", -0.5, 0.5);
	//DrawTriangle();
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);

	GetDynamicRHI()->SetViewPort(0, 0, nWidth, nHeight);
	GET_SHADER_STRUCT_MEMBER(FXAA).Shader_->Use();
	m_FullSizeBuffer->ClearColor();
	Vector4 res = { (float)nWidth, (float)nHeight, 1.0f / nWidth, 1.0f / nHeight };
	Vector2 clip = { -0.5f, 0.5f };
	GET_SHADER_STRUCT_MEMBER(FXAA).SetParameter("resolution", res[0]);
	GET_SHADER_STRUCT_MEMBER(FXAA).SetParameter("clipControl", clip[0]);
	InputTextureRHI->AttatchTextureId(input);
	GET_SHADER_STRUCT_MEMBER(FXAA).SetTexture("colorBuffer", InputTextureRHI);
	DrawTriangle();
	m_FullSizeBuffer->unbind();
}

GLuint PostProcessManagerGL::Bloom(CCFrameBuffer* pFBO)
{
	//glViewport(0, 0, m_HalfSizeBuffer->width, m_HalfSizeBuffer->height);
	//m_HalfSizeBuffer->bind();
	//pScaleShader->Use();
	//glClearColor(0.0, 0, 0, 1.0);
	//glClear(GL_COLOR_BUFFER_BIT);
	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, pFBO->emissID);
	//pScaleShader->SetUniform1i("inputImageTexture", 0);
	//pScaleShader->SetUniform2f("stepSize", 1.0 / pFBO->width, 1.0 / pFBO->height);
	//DrawTriangle();

	GetDynamicRHI()->SetViewPort(0, 0, m_HalfSizeBuffer->width, m_HalfSizeBuffer->height);
	m_HalfSizeBuffer->bind();
	GET_SHADER_STRUCT_MEMBER(Scale).Shader_->Use();
	m_HalfSizeBuffer->ClearColor();
	Vector2 step = { 1.0f / pFBO->width, 1.0f / pFBO->height };
	GET_SHADER_STRUCT_MEMBER(Scale).SetParameter("stepSize", step[0]);
	InputTextureRHI->AttatchTextureId(pFBO->secondID);
	GET_SHADER_STRUCT_MEMBER(Scale).SetTexture("inputImageTexture", InputTextureRHI);
	DrawTriangle();
	m_HalfSizeBuffer->unbind();

	//dual downsample
	//glViewport(0, 0, bloomLevelFBO[0]->width, bloomLevelFBO[0]->height);
	//bloomLevelFBO[0]->bind();
	//glClearColor(0.0, 0, 0, 1.0);
	//glClear(GL_COLOR_BUFFER_BIT);
	//pDualDownSample->Use();
	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, m_HalfSizeBuffer->textureID);
	//pDualDownSample->SetUniform1i("MainTex", 0);

	//pDualDownSample->SetUniform2f("mainTex_texelSize", 0.5 / m_HalfSizeBuffer->width, 0.5 / m_HalfSizeBuffer->height);
	//pDualDownSample->SetUniform1f("_Offset", m_bloomRadius);
	//DrawTriangle();

	GetDynamicRHI()->SetViewPort(0, 0, bloomLevelFBO[0]->width, bloomLevelFBO[0]->height);
	bloomLevelFBO[0]->bind();
	GET_SHADER_STRUCT_MEMBER(DualDowmSample).Shader_->Use();
	bloomLevelFBO[0]->ClearColor();
	Vector2 texelsize = { 0.5f / m_HalfSizeBuffer->width, 0.5f / m_HalfSizeBuffer->height };
	GET_SHADER_STRUCT_MEMBER(DualDowmSample).SetParameter("mainTex_texelSize", texelsize[0]);
	GET_SHADER_STRUCT_MEMBER(DualDowmSample).SetParameter("_Offset", m_bloomRadius);
	InputTextureRHI->AttatchTextureId(m_HalfSizeBuffer->textureID);
	GET_SHADER_STRUCT_MEMBER(DualDowmSample).SetTexture("MainTex", InputTextureRHI);
	DrawTriangle();
	bloomLevelFBO[0]->unbind();


	for (int ni = 1; ni < DUAL_SAMPLE_COUNT; ni++)
	{
		//glViewport(0, 0, bloomLevelFBO[ni]->width, bloomLevelFBO[ni]->height);
		//bloomLevelFBO[ni]->bind();
		//glClearColor(0.0, 0, 0, 1.0);
		//glClear(GL_COLOR_BUFFER_BIT);
		//pDualDownSample->Use();
		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, bloomLevelFBO[ni - 1]->textureID);
		//pDualDownSample->SetUniform1i("MainTex", 0);

		//pDualDownSample->SetUniform2f("mainTex_texelSize", 0.5 / bloomLevelFBO[ni - 1]->width, 0.5 / bloomLevelFBO[ni - 1]->height);
		//pDualDownSample->SetUniform1f("_Offset", m_bloomRadius);
		//DrawTriangle();

		GetDynamicRHI()->SetViewPort(0, 0, bloomLevelFBO[ni]->width, bloomLevelFBO[ni]->height);
		bloomLevelFBO[ni]->bind();
		GET_SHADER_STRUCT_MEMBER(DualDowmSample).Shader_->Use();
		bloomLevelFBO[ni]->ClearColor();
		texelsize = { 0.5f / bloomLevelFBO[ni - 1]->width, 0.5f / bloomLevelFBO[ni - 1]->height };
		GET_SHADER_STRUCT_MEMBER(DualDowmSample).SetParameter("mainTex_texelSize", texelsize[0]);
		GET_SHADER_STRUCT_MEMBER(DualDowmSample).SetParameter("_Offset", m_bloomRadius);
		InputTextureRHI->AttatchTextureId(bloomLevelFBO[ni - 1]->textureID);
		GET_SHADER_STRUCT_MEMBER(DualDowmSample).SetTexture("MainTex", InputTextureRHI);
		DrawTriangle();
	}

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	for (int ni = DUAL_SAMPLE_COUNT - 1; ni > 0; ni--)
	{
		//glViewport(0, 0, bloomLevelFBO[ni -1]->width, bloomLevelFBO[ni - 1]->height);
		//bloomLevelFBO[ni - 1]->bind();

		//pDualUpSample->Use();
		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, bloomLevelFBO[ni]->textureID);
		//pDualUpSample->SetUniform1i("MainTex", 0);

		//pDualUpSample->SetUniform2f("mainTex_texelSize", 0.5 / bloomLevelFBO[ni]->width, 0.5 / bloomLevelFBO[ni]->height);
		//pDualUpSample->SetUniform1f("_Offset", m_bloomRadius);
		//DrawTriangle();

		GetDynamicRHI()->SetViewPort(0, 0, bloomLevelFBO[ni - 1]->width, bloomLevelFBO[ni - 1]->height);
		bloomLevelFBO[ni - 1]->bind();
		GET_SHADER_STRUCT_MEMBER(DualUpSample).Shader_->Use();
		//bloomLevelFBO[ni -1]->ClearColor();
		texelsize = { 0.5f / bloomLevelFBO[ni]->width, 0.5f / bloomLevelFBO[ni]->height };
		GET_SHADER_STRUCT_MEMBER(DualUpSample).SetParameter("mainTex_texelSize", texelsize[0]);
		GET_SHADER_STRUCT_MEMBER(DualUpSample).SetParameter("_Offset", m_bloomRadius);
		InputTextureRHI->AttatchTextureId(bloomLevelFBO[ni]->textureID);
		GET_SHADER_STRUCT_MEMBER(DualUpSample).SetTexture("MainTex", InputTextureRHI);
		DrawTriangle();
	}
	
	//glViewport(0, 0, m_HalfSizeBuffer->width, m_HalfSizeBuffer->height);
	//m_HalfSizeBuffer->bind();
	//pDualUpSample->Use();
	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, bloomLevelFBO[0]->textureID);
	//pDualUpSample->SetUniform1i("MainTex", 0);

	//pDualUpSample->SetUniform2f("mainTex_texelSize", 0.5 / bloomLevelFBO[0]->width, 0.5 / bloomLevelFBO[0]->height);
	//pDualUpSample->SetUniform1f("_Offset", m_bloomRadius);
	//DrawTriangle();

	GetDynamicRHI()->SetViewPort(0, 0, m_HalfSizeBuffer->width, m_HalfSizeBuffer->height);
	m_HalfSizeBuffer->bind();
	GET_SHADER_STRUCT_MEMBER(DualUpSample).Shader_->Use();
	m_HalfSizeBuffer->ClearColor();
	texelsize = { 0.5f / bloomLevelFBO[0]->width, 0.5f / bloomLevelFBO[0]->height };
	GET_SHADER_STRUCT_MEMBER(DualUpSample).SetParameter("mainTex_texelSize", texelsize[0]);
	GET_SHADER_STRUCT_MEMBER(DualUpSample).SetParameter("_Offset", m_bloomRadius);
	InputTextureRHI->AttatchTextureId(bloomLevelFBO[0]->textureID);
	GET_SHADER_STRUCT_MEMBER(DualUpSample).SetTexture("MainTex", InputTextureRHI);
	DrawTriangle();

	glDisable(GL_BLEND);

	//glViewport(0, 0, pFBO->width,pFBO->height);
	//m_FBO_Ext->bind();
	//GLuint attachments1[1] = { GL_COLOR_ATTACHMENT0 };
	//glDrawBuffers(1, attachments1);

	//pShader->Use();
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	//glColorMask(1, 1, 1, 0);
	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, m_HalfSizeBuffer->textureID);
	//pShader->SetUniform1i("inputImageTexture", 0);
	//pShader->SetUniform1f("alpha", m_bloomAlpha);
	//DrawTriangle();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glColorMask(1, 1, 1, 0);

	GetDynamicRHI()->SetViewPort(0, 0, pFBO->width, pFBO->height);
	m_FBO_Ext->bind();
	m_FBO_Ext->AttatchColorBuffers(1);
	GET_SHADER_STRUCT_MEMBER(Quad2D).Shader_->Use();
	GET_SHADER_STRUCT_MEMBER(Quad2D).SetParameter("alpha", m_bloomAlpha);
	InputTextureRHI->AttatchTextureId(m_HalfSizeBuffer->textureID);
	GET_SHADER_STRUCT_MEMBER(Quad2D).SetTexture("inputImageTexture", InputTextureRHI);
	DrawTriangle();

	glColorMask(1, 1, 1, 1);
	glDisable(GL_BLEND);

	return m_FBO_Ext->textureID;
}

void PostProcessManagerGL::DrawTriangle()
{
	//glBindVertexArray(mVao);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEbo);

	//glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, 0);

	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	//glBindVertexArray(0);

	GetDynamicRHI()->DrawPrimitive(postProcessTriMesh, postProcessIndexBuffer);
}



void PostProcessManagerGL::SetShaderResource(const std::string& path)
{
	if (pFxaaShader == nullptr)
	{
		pFxaaShader = std::make_shared<CCProgram>();
		std::string vs_depth = path + "/Shader/3D/fxaa.vs";
		std::string fs_depth = path + "/Shader/3D/fxaa.fs";
		pFxaaShader->CreatePorgramForFile(vs_depth.c_str(), fs_depth.c_str());
		GET_SHADER_STRUCT_MEMBER(FXAA).Shader_ = pFxaaShader;
	}

	if (pScaleShader == nullptr)
	{
		pScaleShader = std::make_shared<CCProgram>();
		std::string vs_depth = path + "/Shader/3D/scale.vs";
		std::string fs_depth = path + "/Shader/3D/scale.fs";
		pScaleShader->CreatePorgramForFile(vs_depth.c_str(), fs_depth.c_str());
		GET_SHADER_STRUCT_MEMBER(Scale).Shader_ = pScaleShader;
	}

	//if (pBulrShader == nullptr)
	//{
	//	pBulrShader = std::make_shared<CCProgram>();
	//	std::string vs_depth = path + "/Shader/3D/Gaussianblur.vs";
	//	std::string fs_depth = path + "/Shader/3D/Gaussianblur.fs";
	//	pBulrShader->CreatePorgramForFile(vs_depth.c_str(), fs_depth.c_str());
	//}

	if (pShader == nullptr)
	{
		pShader = std::make_shared<CCProgram>();
		std::string vs_depth = path + "/Shader/3D/face2dTexture.vs";
		std::string fs_depth = path + "/Shader/3D/face2dTexture.fs";
		pShader->CreatePorgramForFile(vs_depth.c_str(), fs_depth.c_str());
		GET_SHADER_STRUCT_MEMBER(Quad2D).Shader_ = pShader;

	}

	if (pDualDownSample == nullptr)
	{
		pDualDownSample = std::make_shared<CCProgram>();
		std::string vs_depth = path + "/Shader/3D/dualblur_downsample.vs";
		std::string fs_depth = path + "/Shader/3D/dualblur_downsample.fs";
		pDualDownSample->CreatePorgramForFile(vs_depth.c_str(), fs_depth.c_str());
		GET_SHADER_STRUCT_MEMBER(DualDowmSample).Shader_ = pDualDownSample;
	}

	if (pDualUpSample == nullptr)
	{
		pDualUpSample = std::make_shared<CCProgram>();
		std::string vs_depth = path + "/Shader/3D/dualblur_upsample.vs";
		std::string fs_depth = path + "/Shader/3D/dualblur_upsample.fs";
		pDualUpSample->CreatePorgramForFile(vs_depth.c_str(), fs_depth.c_str());
		GET_SHADER_STRUCT_MEMBER(DualUpSample).Shader_ = pDualUpSample;
	}
}

