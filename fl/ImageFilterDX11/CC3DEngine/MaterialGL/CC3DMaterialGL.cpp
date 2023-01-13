
#include "CC3DMaterialGL.h"
#include "Model/CC3DMesh.h"
#include "ToolBox/GL/GLResource.h"
#include "Toolbox/Render/MaterialTexRHI.h"

CC3DMaterialGL::CC3DMaterialGL()
{
	m_OcclusionTexture = NULL;
	m_NormalTexture = NULL;
	m_MetallicRoughnessTexture = NULL;
	m_BaseColorTexture = NULL;
	m_EmissiveTexture = NULL;
	alphaMode = "OPAQUE";
	doubleSided = false;
	alphaCutoff = 0.5;
	metallicFactor = 0.5;
	roughnessFactor = 0.5;
}

CC3DMaterialGL::~CC3DMaterialGL()
{

}

void CC3DMaterialGL::InitMaterial(uint32 MaterialIndex, const std::vector<std::shared_ptr<CC3DTextureRHI>> &ModelTexture)
{
	const tinygltf::Material &material = m_Model->materials[MaterialIndex];

	MaterialName = material.name;
	alphaMode = material.alphaMode;
	doubleSided = material.doubleSided;
	alphaCutoff = material.alphaCutoff;
	metallicFactor = material.pbrMetallicRoughness.metallicFactor;
	roughnessFactor = material.pbrMetallicRoughness.roughnessFactor;

	auto &gltfTexture = m_Model->textures;
	int32 index = material.pbrMetallicRoughness.baseColorTexture.index;
	if (index<0)
	{
		auto extensions = material.extensions;
		auto it = extensions.begin();
		if (it != extensions.end())
		{
			auto value = it->second;
			auto vIndex = value.Get("diffuseTexture");
			if (vIndex.Has("index"))
			{
				index = vIndex.Get("index").Get<int>();
			}
	

		}
	}
	if (index>-1&&index< gltfTexture.size()&& gltfTexture[index].source<ModelTexture.size())
	{
		m_BaseColorTexture = ModelTexture[gltfTexture[index].source];
	}
	else
	{
		auto &BaseColor = material.pbrMetallicRoughness.baseColorFactor;
		m_BaseColorTexture = GetDynamicRHI()->CreateTexture((float)BaseColor[0], (float)BaseColor[1], (float)BaseColor[2], (float)BaseColor[3]);
	}

	index = material.pbrMetallicRoughness.metallicRoughnessTexture.index;
	if (index > -1 && index < gltfTexture.size() && gltfTexture[index].source < ModelTexture.size())
	{
		m_MetallicRoughnessTexture = ModelTexture[gltfTexture[index].source];
	}
	else
	{
		m_MetallicRoughnessTexture = GetDynamicRHI()->CreateTexture(1.0f, roughnessFactor, metallicFactor, 1.0f);
	}
	index = material.emissiveTexture.index;
	if (index > -1 && index < gltfTexture.size() && gltfTexture[index].source < ModelTexture.size())
	{
		m_EmissiveTexture = ModelTexture[gltfTexture[index].source];
	}

	else
	{
		m_EmissiveTexture = GetDynamicRHI()->CreateTexture(0.0f, 0.0f, 0.0f, 1.0f);
	}

	index = material.normalTexture.index;
	if (index > -1 && index < gltfTexture.size() && gltfTexture[index].source < ModelTexture.size())
	{
		m_NormalTexture = ModelTexture[gltfTexture[index].source];
	}
	else
	{
		m_NormalTexture = GetDynamicRHI()->CreateTexture(0.5f, 0.5f, 1.0f, 1.0f);
	}

	index = material.occlusionTexture.index;
	if (index > -1 && index < gltfTexture.size() && gltfTexture[index].source < ModelTexture.size())
	{
		m_OcclusionTexture = ModelTexture[gltfTexture[index].source];
	}
	else
	{
		m_OcclusionTexture = GetDynamicRHI()->CreateTexture(1.0f, 1.0f, 1.0f, 1.0f);
	}
}

void CC3DMaterialGL::CreateDefault()
{
	m_NormalTexture = GetDynamicRHI()->CreateTexture(0.5f, 0.5f, 1.0f, 1.0f);

	{
		std::shared_ptr<CC3DTextureRHI> TexRHI = GetDynamicRHI()->CreateTexture(0.5f, 0.5f, 0.5f, 1.0f);
		m_BaseColorTexture = TexRHI;
		m_MetallicRoughnessTexture = TexRHI;

	}

	m_EmissiveTexture = GetDynamicRHI()->CreateTexture(0.f, 0.f, 0.f, 1.0f);
}

void CC3DMaterialGL::InitShaderProgram(std::string path)
{
	//if (m_pShaderPBR == nullptr)
	//{
	//	m_pShaderPBR = new CCProgram();
	//	std::string  vspath = path + "/Shader/pbrTex.vs";
	//	std::string  fspath = path + "/Shader/pbrTex.fs";
	//	m_pShaderPBR->CreatePorgramForFile(vspath.c_str(), fspath.c_str());
	//}

	//if (m_pShaderFur == nullptr)
	//{
	//	m_pShaderFur = new CCProgram();
	//	std::string  vspath = path + "/Shader/fur.vs";
	//	std::string  fspath = path + "/Shader/fur.fs";
	//	m_pShaderFur->CreatePorgramForFile(vspath.c_str(), fspath.c_str());
	//}
}

void CC3DMaterialGL::UseProgram()
{
	if (pShader)
	{
		pShader->Use();
	}
	if (mShader)
	{
		mShader->UseShader();
	}
}

void CC3DMaterialGL::UseShader()
{
	if (mShader)
	{
		mShader->UseShader();
	}
}

void CC3DMaterialGL::PreRenderSet(CC3DMesh* pMesh)
{

}

void CC3DMaterialGL::RenderSet(CC3DMesh* pMesh)
{

}

void CC3DMaterialGL::SetParams(const std::string& Params)
{

}

void CC3DMaterialGL::DrawTriangle(CC3DMesh * pMesh)
{
	//glBindVertexArray(pMesh->m_pGPUBuffer->m_VAO);

	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pMesh->m_pGPUBuffer->m_EBO);
	//if (pMesh->m_mesh->pFacesIndex != NULL)
	//{
	//	glDrawElements(GL_TRIANGLES, pMesh->m_mesh->nNumFaces * 3, GL_UNSIGNED_SHORT, 0);
	//}
	//else
	//{
	//	glDrawElements(GL_TRIANGLES, pMesh->m_mesh->nNumFaces * 3, GL_UNSIGNED_INT, 0);
	//}
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	//glBindVertexArray(0);

	GetDynamicRHI()->DrawPrimitive(pMesh->m_pGPUBuffer->VerticeBuffer[0], pMesh->m_pGPUBuffer->IndexBuffer);
}

void CC3DMaterialGL::SetUniformMatrix4fv(const char *str, const GLfloat *array, bool transpose, int count)
{
	
	pShader->SetUniformMatrix4fv(str, array, transpose, count);
}

void CC3DMaterialGL::SetUniform3f(const char * str, float x, float y, float z)
{
	pShader->SetUniform3f(str, x, y, z);
}

void CC3DMaterialGL::SetUniform4f(const char* str, float x, float y, float z, float w)
{
	pShader->SetUniform4f(str, x, y, z, w);
}

void CC3DMaterialGL::SetUniform1f(const char* str, float value)
{
	pShader->SetUniform1f(str, value);
}

void CC3DMaterialGL::SetUniform1i(const char* str, int value)
{
	pShader->SetUniform1i(str, value);
}

void CC3DMaterialGL::SetTexture2D(const char* str, GLuint id)
{

}

void CC3DMaterialGL::SetTexture2D(const char* str, std::shared_ptr<CC3DTextureRHI> TexRHI)
{
	auto itr = texture_map.find(str);
	if (itr != texture_map.end())
	{
		int texture_index = itr->second;
		glActiveTexture(GL_TEXTURE0 + texture_index);
		glBindTexture(GL_TEXTURE_2D, RHIResourceCast(TexRHI.get())->GetTextureID());
		pShader->SetUniform1i(str, texture_index);
	}
}

void CC3DMaterialGL::SetTexture2D(const char* str, std::shared_ptr<CC3DRenderTargetRHI> TexRHI)
{
	auto itr = texture_map.find(str);
	if (itr != texture_map.end())
	{
		int texture_index = itr->second;
		glActiveTexture(GL_TEXTURE0 + texture_index);
		glBindTexture(GL_TEXTURE_2D, RHIResourceCast(TexRHI.get())->GetTextureId());
		pShader->SetUniform1i(str, texture_index);
	}
}

void CC3DMaterialGL::SetTexture2D(const char* str, std::shared_ptr<CC3DCubeMapRHI> CubeMapRHI)
{
	
}

void CC3DMaterialGL::SetTexture2D(const char* str, std::shared_ptr<MaterialTexRHI> TexRHI)
{
	auto itr = texture_map.find(str);
	if (itr != texture_map.end())
	{
		int texture_index = itr->second;
		TexRHI->Bind(texture_index);
		pShader->SetUniform1i(str, texture_index);
	}
}

void CC3DMaterialGL::SetCubeMap(const char* str, GLuint id)
{

}

void CC3DMaterialGL::SetCubeMap(const char* str, std::shared_ptr<CC3DCubeMapRHI> CubeMapRHI)
{
	auto itr = texture_map.find(str);
	if (itr != texture_map.end())
	{
		int texture_index = itr->second;
		glActiveTexture(GL_TEXTURE0 + texture_index);
		glBindTexture(GL_TEXTURE_CUBE_MAP, RHIResourceCast(CubeMapRHI.get())->GetTextureId());
		pShader->SetUniform1i(str, texture_index);
	}
}

void CC3DMaterialGL::LoadConfig(const std::string config_file)
{

}

void CC3DMaterialGL::SetBoneMatrix(const glm::mat4& mat, int index)
{
	GET_SHADER_STRUCT_MEMBER(PBRSkinMat).SetArraySingleElementParamter("gBonesMatrix", mat, index);
}

