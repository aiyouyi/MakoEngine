#include "CC3DAdvancedEyeMaterial.h"
#include "RenderGL/ShaderProgramManagerGL.h"


CC3DAdvancedEyeMaterial::CC3DAdvancedEyeMaterial()
{
	materialType = MaterialTypeGL::EYE;
}

CC3DAdvancedEyeMaterial::~CC3DAdvancedEyeMaterial()
{

}

void CC3DAdvancedEyeMaterial::InitShaderProgram(std::string path)
{
	std::string  vspath = path + "/Shader/3D/advancedEye.vs";
	std::string  fspath = path + "/Shader/3D/advancedEye.fs";
	pShader = ShaderProgramManagerGL::GetInstance()->GetOrCreateShaderByPath(vspath, fspath);

	irisColorTex	 = GetDynamicRHI()->CreateTextureFromFile("", false);
	irisTex			 = GetDynamicRHI()->CreateTextureFromFile("", false);
	mainTex			 = GetDynamicRHI()->CreateTextureFromFile("", false);
	shadeScleraTex	 = GetDynamicRHI()->CreateTextureFromFile("", false);
	shadeIrisTex	 = GetDynamicRHI()->CreateTextureFromFile("", false);
	corneaBump		 = GetDynamicRHI()->CreateTextureFromFile("", false);
	eyeBump			 = GetDynamicRHI()->CreateTextureFromFile("", false);
	irisBump		 = GetDynamicRHI()->CreateTextureFromFile("", false);
}

void CC3DAdvancedEyeMaterial::RenderSet(CC3DMesh* pMesh)
{
	//if (pShader)
	//{
	//	glActiveTexture(GL_TEXTURE0);
	//	glBindTexture(GL_TEXTURE_2D, irisColorTex->GetGLTextureID());
	//	pShader->SetUniform1i("_IrisColorTex", 0);
	//	glActiveTexture(GL_TEXTURE1);
	//	glBindTexture(GL_TEXTURE_2D, irisTex->GetGLTextureID());
	//	pShader->SetUniform1i("_IrisTex", 1);
	//	glActiveTexture(GL_TEXTURE2);
	//	glBindTexture(GL_TEXTURE_2D, mainTex->GetGLTextureID());
	//	pShader->SetUniform1i("_MainTex", 2);
	//	glActiveTexture(GL_TEXTURE3);
	//	glBindTexture(GL_TEXTURE_2D, shadeScleraTex->GetGLTextureID());
	//	pShader->SetUniform1i("_ShadeScleraTex", 3);
	//	glActiveTexture(GL_TEXTURE4);
	//	glBindTexture(GL_TEXTURE_2D, shadeIrisTex->GetGLTextureID());
	//	pShader->SetUniform1i("_ShadeIrisTex", 4);
	//	glActiveTexture(GL_TEXTURE5);
	//	glBindTexture(GL_TEXTURE_2D, corneaBump->GetGLTextureID());
	//	pShader->SetUniform1i("_CorneaBump", 5);
	//	glActiveTexture(GL_TEXTURE6);
	//	glBindTexture(GL_TEXTURE_2D, eyeBump->GetGLTextureID());
	//	pShader->SetUniform1i("_EyeBump", 6);
	//	glActiveTexture(GL_TEXTURE7);
	//	glBindTexture(GL_TEXTURE_2D, irisBump->GetGLTextureID());
	//	pShader->SetUniform1i("_IrisBump", 7);

	//	SetParameter("_scleraShadowAmt", _scleraShadowAmt);
	//	SetParameter("_irisShadowAmt", _irisShadowAmt);
	//	SetParameter("_irisSize", _irisSize);
	//	SetParameter("_scleraSize", _scleraSize);
	//	SetParameter("_limbus", _limbus);
	//	SetParameter("_parallax", _parallax);
	//	SetParameter("_smoothness", _smoothness);
	//	SetParameter("_specsize", _specsize);
	//	SetParameter("_reflectTerm", _reflectTerm);

	//	SetParameter("_scleraColor", _scleraColor);
	//	SetParameter("_irisColor", _irisColor);
	//	SetParameter("_illumColor", _illumColor);


	//	DrawTriangle(pMesh);
	//}

	SetTexture2D("_IrisColorTex", irisColorTex);
	SetTexture2D("_IrisTex", irisTex);
	SetTexture2D("_MainTex", mainTex);
	SetTexture2D("_ShadeScleraTex", shadeScleraTex);
	SetTexture2D("_ShadeIrisTex", shadeIrisTex);
	SetTexture2D("_CorneaBump", corneaBump);
	SetTexture2D("_EyeBump", eyeBump);
	SetTexture2D("_IrisBump", irisBump);

	SetParameter("_scleraShadowAmt", _scleraShadowAmt);
	SetParameter("_irisShadowAmt", _irisShadowAmt);
	SetParameter("_irisSize", _irisSize);
	SetParameter("_scleraSize", _scleraSize);
	SetParameter("_limbus", _limbus);
	SetParameter("_parallax", _parallax);
	SetParameter("_smoothness", _smoothness);
	SetParameter("_specsize", _specsize);
	SetParameter("_reflectTerm", _reflectTerm);

	SetParameter("_scleraColor", _scleraColor);
	SetParameter("_irisColor", _irisColor);
	SetParameter("_illumColor", _illumColor);

	DrawTriangle(pMesh);
}

void CC3DAdvancedEyeMaterial::SetTexture2D(const char* str, std::shared_ptr<CC3DTextureRHI> TextureRHI)
{
	GET_SHADER_STRUCT_MEMBER(AdvancedEye).SetTexture(str, TextureRHI);
}

void CC3DAdvancedEyeMaterial::SetTexture2D(const char* str, std::shared_ptr<CC3DRenderTargetRHI> RenderTargetRHI)
{
	GET_SHADER_STRUCT_MEMBER(AdvancedEye).SetTexture(str, RenderTargetRHI);
}

void CC3DAdvancedEyeMaterial::SetTexture2D(const char* str, std::shared_ptr<CC3DCubeMapRHI> TextureRHI)
{
	GET_SHADER_STRUCT_MEMBER(AdvancedEye).SetTexture(str, TextureRHI);
}

void CC3DAdvancedEyeMaterial::SetParams(const std::string& Params)
{
	
}


void CC3DAdvancedEyeMaterial::LoadConfig(const std::string config_file)
{

}

