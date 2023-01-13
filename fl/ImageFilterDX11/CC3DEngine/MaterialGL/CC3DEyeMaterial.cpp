#include "CC3DEyeMaterial.h"
//#include "RenderGL/ShaderProgramManagerGL.h"

#include "Model/CC3DMesh.h"
//#include "ToolBox/GL/GLResource.h"
#include "Toolbox/Render/DynamicRHI.h"
#include "Toolbox/RenderState/PiplelineState.h"
#include "Toolbox/Render/MaterialTexRHI.h"

CC3DEyeMaterial::CC3DEyeMaterial()
{
	materialType == MaterialType::PBR;
// 	texture_map = { {"irradianceMap", 3},
// 	{ "brdfLUT", 4 },
// 	{ "prefilterMap",5},
// 	};
}

CC3DEyeMaterial::~CC3DEyeMaterial()
{

}

void CC3DEyeMaterial::InitShaderProgram(const std::string& path, CC3DImageFilter::EffectConfig* EffectConfig)
{
	if (mShader)
	{
		return;
	}
	mShader = GetDynamicRHI()->CreateShaderRHI();
	if (GetDynamicRHI()->API == CC3DDynamicRHI::DX11)
	{

	}
	else
	{
		std::string  vspath = path + "/Shader/3D/eyeDoll.vs";
		std::string  fspath = path + "/Shader/3D/eyeDoll.fs";
		mShader->InitShader(vspath.c_str(), fspath.c_str() );
		GET_SHADER_STRUCT_MEMBER(PBRConstantBuffer).Shader_ = mShader->GetGLProgram();
		GET_SHADER_STRUCT_MEMBER(PBRSkinMat).Shader_ = mShader->GetGLProgram();
		GET_SHADER_STRUCT_MEMBER(EyeDoll).Shader_ = mShader->GetGLProgram();
	}



	//std::string  vspath = path + "/Shader/3D/eyeDoll.vs";
	//std::string  fspath = path + "/Shader/3D/eyeDoll.fs";
	//pShader = ShaderProgramManagerGL::GetInstance()->GetOrCreateShaderByPath(vspath, fspath);
	m_BaseColorTexture = GetDynamicRHI()->CreateTextureFromFile("./3DModel/test3/eye/eye_d3.jpg");
	specularMap		   = GetDynamicRHI()->CreateTextureFromFile( "./3DModel/test3/eye/cook.jpg");
	heightMap		   = GetDynamicRHI()->CreateTextureFromFile( "./3DModel/test3/eye/parralx2.PNG");
	//specularMap = new CC3DTexture();
	//specularMap->LoadTexture("./3DModel/test3/eye/cook.jpg");
	//heightMap = new CC3DTexture();
	//heightMap->LoadTexture("./3DModel/test3/eye/parralx2.PNG");
}

void CC3DEyeMaterial::RenderSet(CC3DMesh* pMesh)
{
	//if (pShader != nullptr)
	//{
	//	glActiveTexture(GL_TEXTURE0);
	//	glBindTexture(GL_TEXTURE_2D, RHIResourceCast(m_BaseColorTexture.get())->GetTextureID());
	//	pShader->SetUniform1i("albedoMap", 0);
	//	glActiveTexture(GL_TEXTURE1);
	//	glBindTexture(GL_TEXTURE_2D, specularMap->GetGLTextureID());
	//	pShader->SetUniform1i("specularMap", 1);
	//	glActiveTexture(GL_TEXTURE2);
	//	glBindTexture(GL_TEXTURE_2D, heightMap->GetGLTextureID());
	//	pShader->SetUniform1i("heightMap", 2);

	//	pShader->SetUniform3f("specoffset1", specoffset1.x, specoffset1.y, specoffset1.z);
	//	pShader->SetUniform3f("specoffset2", specoffset2.x, specoffset2.y, specoffset2.z);
	//	pShader->SetUniform3f("specoffset1", specoffset3.x, specoffset3.y, specoffset3.z);
	//	pShader->SetUniform3f("specoffset1", initoffset.x, initoffset.y, initoffset.z);
	//	pShader->SetUniform3f("specoffset1", intensity.x, intensity.y, intensity.z);

	//	pShader->SetUniform1f("movespeed1", movespeed1);
	//	pShader->SetUniform1f("movespeed2", movespeed2);
	//	pShader->SetUniform1f("movespeed3", movespeed3);
	//	pShader->SetUniform1f("eyeheight", eyeheight);

	//}
	//DrawTriangle(pMesh);

	SetTexture2D("albedoMap", m_BaseColorTexture);
	SetTexture2D("specularMap", specularMap);
	SetTexture2D("heightMap", heightMap);
	GetDynamicRHI()->SetSamplerState(CC3DPiplelineState::MirrorLinerSampler);
	SetParameter("specoffset1", specoffset1);
	SetParameter("specoffset2", specoffset2);
	SetParameter("specoffset3", specoffset3);
	SetParameter("initoffset", initoffset);
	SetParameter("intensity", intensity);

	SetParameter("movespeed1", movespeed1);
	SetParameter("movespeed2", movespeed2);
	SetParameter("movespeed3", movespeed3);
	SetParameter("eyeheight", eyeheight);

	GET_SHADER_STRUCT_MEMBER(PBRConstantBuffer).ApplyToAllBuffer();
	GET_SHADER_STRUCT_MEMBER(PBRSkinMat).ApplyToVSBuffer();
	GET_SHADER_STRUCT_MEMBER(EyeDoll).ApplyToAllBuffer();

	DrawTriangle(pMesh);
}

void CC3DEyeMaterial::SetTexture2D(const char* str, std::shared_ptr<CC3DTextureRHI> TextureRHI)
{
	GET_SHADER_STRUCT_MEMBER(EyeDoll).SetTexture(str, TextureRHI);
}

void CC3DEyeMaterial::SetTexture2D(const char* str, std::shared_ptr<CC3DRenderTargetRHI> RenderTargetRHI)
{
	GET_SHADER_STRUCT_MEMBER(EyeDoll).SetTexture(str, RenderTargetRHI);
}

void CC3DEyeMaterial::SetTexture2D(const char* str, std::shared_ptr<CC3DCubeMapRHI> TextureRHI)
{
	GET_SHADER_STRUCT_MEMBER(EyeDoll).SetTexture(str, TextureRHI);
}

void CC3DEyeMaterial::SetTexture2D(const char* str, std::shared_ptr<MaterialTexRHI> TextureRHI)
{
	TextureRHI->UpdateTexture();
	GET_SHADER_STRUCT_MEMBER(EyeDoll).SetTexture(str, TextureRHI->GetTextureRHI());
}

void CC3DEyeMaterial::SetParams(const std::string& Params)
{

}
