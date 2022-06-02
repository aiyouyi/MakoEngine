
#include "CC3DMaterial.h"
#include "Model/CC3DMesh.h"
#include "Toolbox/DXUtils/DX11Shader.h"
#include "Toolbox/Render/TextureRHI.h"

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

void CC3DMaterial::InitMaterial(uint32 MaterialIndex, std::vector<std::shared_ptr<CC3DTextureRHI>>&ModelTexture)
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

		std::shared_ptr<CC3DTextureRHI> TextureRHI = GetDynamicRHI()->CreateTexture((float)BaseColor[0], (float)BaseColor[1], (float)BaseColor[2], (float)BaseColor[3]);
		m_ModelTexture.push_back(TextureRHI);
		m_BaseColorTexture = TextureRHI;
	}

	index = material.pbrMetallicRoughness.metallicRoughnessTexture.index;
	if (index > -1 && index < gltfTexture.size() && gltfTexture[index].source < ModelTexture.size())
	{
		m_MetallicRoughnessTexture = ModelTexture[gltfTexture[index].source];
	}
	else
	{
		std::shared_ptr<CC3DTextureRHI> TextureRHI = GetDynamicRHI()->CreateTexture(1.0, roughnessFactor, metallicFactor, 1.0);
		m_ModelTexture.push_back(TextureRHI);
		m_MetallicRoughnessTexture = TextureRHI;
	}
	index = material.emissiveTexture.index;
	if (index > -1 && index < gltfTexture.size() && gltfTexture[index].source < ModelTexture.size())
	{
		m_EmissiveTexture = ModelTexture[gltfTexture[index].source];
	}

	else
	{
		auto emissiveColor = material.emissiveFactor;
		std::shared_ptr<CC3DTextureRHI> TextureRHI = GetDynamicRHI()->CreateTexture((float)emissiveColor[0], (float)emissiveColor[1], (float)emissiveColor[2], 1.0);
		m_ModelTexture.push_back(TextureRHI);
		m_EmissiveTexture = TextureRHI;
	}

	index = material.normalTexture.index;
	if (index > -1 && index < gltfTexture.size() && gltfTexture[index].source < ModelTexture.size())
	{
		m_NormalTexture = ModelTexture[gltfTexture[index].source];
	}
	else
	{
		std::shared_ptr<CC3DTextureRHI> TextureRHI = GetDynamicRHI()->CreateTexture(0.5f, 0.5f, 1.0f, 1.0f);
		m_ModelTexture.push_back(TextureRHI);
		m_NormalTexture = TextureRHI;
	}

	index = material.occlusionTexture.index;
	if (index > -1 && index < gltfTexture.size() && gltfTexture[index].source < ModelTexture.size())
	{
		m_OcclusionTexture = ModelTexture[gltfTexture[index].source];
	}
	else
	{
		std::shared_ptr<CC3DTextureRHI> TextureRHI = GetDynamicRHI()->CreateTexture(1.0f, 1.0f, 1.0f, 1.0f);
		m_ModelTexture.push_back(TextureRHI);
		m_OcclusionTexture = TextureRHI;
	}

}

void CC3DMaterial::CreateDefault()
{

	std::shared_ptr<CC3DTextureRHI> TextureRHI1 = GetDynamicRHI()->CreateTexture(0.5f, 0.5f, 1.0f, 1.0f);
	m_ModelTexture.push_back(TextureRHI1);
	m_NormalTexture = TextureRHI1;

	std::shared_ptr<CC3DTextureRHI> TextureRHI2 = GetDynamicRHI()->CreateTexture(0.5f, 0.5f, 0.5f, 1.0f);
	m_ModelTexture.push_back(TextureRHI2);
	m_BaseColorTexture = TextureRHI2;
	m_MetallicRoughnessTexture = TextureRHI2;


	std::shared_ptr<CC3DTextureRHI> TextureRHI3 = GetDynamicRHI()->CreateTexture(0.0f, 0.0f, 0.0f, 1.0f);
	m_ModelTexture.push_back(TextureRHI3);
	m_EmissiveTexture = TextureRHI3;

}


void CC3DMaterial::InitShaderProgram(std::string path)
{

}

void CC3DMaterial::UseShader()
{
	if (pShader)
	{
		pShader->useShader();
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


void CC3DMaterial::DrawTriangle(CC3DMesh* pMesh)
{
	GetDynamicRHI()->DrawPrimitive(pMesh->m_pGPUBuffer->VerticeBuffer, pMesh->m_pGPUBuffer->AtrributeCount, pMesh->m_pGPUBuffer->IndexBuffer);
}

void CC3DMaterial::SetBoneMatrix(glm::mat4 mat, int index)
{
	GET_CONSTBUFFER(PBRSkinMat).BoneMat[index] = mat;
}
