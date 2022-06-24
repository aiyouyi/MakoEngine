#include "RenderPrimitive.h"
#include"GLBMeshData.h"


inline RenderModel::~RenderModel()
{
	ClearMesh();
}

void RenderModel::LoadMeshFromFile(std::wstring FilePath)
{
	ClearMesh();
	auto GLBMesh=GLBMeshPool::GetInstance()->GetGLBMesh(FilePath);
	if (GLBMesh)
	{
		for (int IndexMesh = 0; IndexMesh < GLBMesh->MeshNum(); ++IndexMesh)
		{
			RenderMesh* pMesh = new RenderMesh();
			for (int IndexPrim = 0; IndexPrim < GLBMesh->PrimitiveNum(IndexMesh); ++IndexPrim)
			{
				RenderPrimitive* pRenderPrimitive = new RenderPrimitive();
				pRenderPrimitive->InitData(GLBMesh);
				pMesh->Primitives.push_back(pRenderPrimitive);
			}
			Meshes.push_back(pMesh);
		}
	}
}

void RenderModel::ClearMesh()
{
	for (auto Iter : Meshes)
	{
		delete Iter;
	}
	Meshes.clear();
}

inline RenderMesh::~RenderMesh()
{
	ClearPrimitive();
}

void RenderMesh::ClearPrimitive()
{
	for (auto Iter : Primitives)
	{
		delete Iter;
	}
	Primitives.clear();
}

bool RenderPrimitive::InitData(GLBMeshData* pGLBMeshData)
{
	return false;
}
