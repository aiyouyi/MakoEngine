
#include "CC3DMaterial.h"
#include "Model/CC3DMesh.h"
#include "Toolbox/DXUtils/DX11Shader.h"
#include "Toolbox/Render/TextureRHI.h"
#include "Toolbox/Render/ShaderRHI.h"
#include "Toolbox/Render/MaterialTexRHI.h"

CC3DMaterial::CC3DMaterial()
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

CC3DMaterial::~CC3DMaterial()
{
}

void CC3DMaterial::InitMaterial(uint32 MaterialIndex, const std::vector<std::shared_ptr<CC3DTextureRHI>>&ModelTexture)
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
		m_MetallicRoughnessTexture = GetDynamicRHI()->CreateTexture(1.0, roughnessFactor, metallicFactor, 1.0);
	}
	index = material.emissiveTexture.index;
	if (index > -1 && index < gltfTexture.size() && gltfTexture[index].source < ModelTexture.size())
	{
		m_EmissiveTexture = ModelTexture[gltfTexture[index].source];
	}

	else
	{
		auto emissiveColor = material.emissiveFactor;
		m_EmissiveTexture = GetDynamicRHI()->CreateTexture((float)emissiveColor[0], (float)emissiveColor[1], (float)emissiveColor[2], 1.0);
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

void CC3DMaterial::CreateDefault()
{

	m_NormalTexture = GetDynamicRHI()->CreateTexture(0.5f, 0.5f, 1.0f, 1.0f);

	auto TextureRHI2 = GetDynamicRHI()->CreateTexture(0.5f, 0.5f, 0.5f, 1.0f);
	m_BaseColorTexture = TextureRHI2;
	m_MetallicRoughnessTexture = TextureRHI2;

	m_EmissiveTexture = GetDynamicRHI()->CreateTexture(0.0f, 0.0f, 0.0f, 1.0f);
}


void CC3DMaterial::InitShaderProgram(const std::string& path, CC3DImageFilter::EffectConfig* EffectConfig)
{

}

void CC3DMaterial::UseShader()
{

	if (mShader)
	{
		mShader->UseShader();
	}
}

void CC3DMaterial::PreRenderSet(CC3DMesh* pMesh)
{

}

void CC3DMaterial::RenderSet(CC3DMesh* pMesh)
{

}


void CC3DMaterial::SetTexture2D(const char* str, std::shared_ptr<CC3DRenderTargetRHI> RenderTargetRHI)
{

}

void CC3DMaterial::SetTexture2D(const char* str, std::shared_ptr<CC3DTextureRHI> TextureRHI)
{

}

void CC3DMaterial::SetTexture2D(const char* str, std::shared_ptr<CC3DCubeMapRHI> TextureRHI)
{

}

void CC3DMaterial::SetTexture2D(const char* str, std::shared_ptr<MaterialTexRHI> TextureRHI)
{

}

void CC3DMaterial::DrawTriangle(CC3DMesh* pMesh)
{
	GetDynamicRHI()->DrawPrimitive(pMesh->m_pGPUBuffer->VerticeBuffer, pMesh->m_pGPUBuffer->AtrributeCount, pMesh->m_pGPUBuffer->IndexBuffer);
}

void CC3DMaterial::SetBoneMatrix(const glm::mat4& mat, int index)
{
	GET_SHADER_STRUCT_MEMBER(PBRSkinMat).SetArraySingleElementParamter("gBonesMatrix", mat, index);
}
