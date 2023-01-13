
#include "CC3DMeshBuffer.h"
#include <fstream>
#include "CC3DMeshBuffer.h"
#include "Toolbox/Render/VertexBuffer.h"

CC3DMeshBuffer::CC3DMeshBuffer()
{
}

CC3DMeshBuffer::~CC3DMeshBuffer()
{
}

void CC3DMeshBuffer::InitMesh(CC3DMeshInfo * pMesh)
{
	if (pMesh == nullptr)
	{
		return;
	}

	if (GetDynamicRHI()->API == CC3DDynamicRHI::DX11)
	{
		VerticeBuffer[0] = GetDynamicRHI()->CreateVertexBuffer((float*)pMesh->pVertices, pMesh->nNumVertices, 3);
		VerticeBuffer[1] = GetDynamicRHI()->CreateVertexBuffer((float*)pMesh->pNormals, pMesh->nNumVertices, 3);
		if (pMesh->pTextureCoords == nullptr)
		{
			VerticeBuffer[2] = GetDynamicRHI()->CreateVertexBuffer((float*)pMesh->pNormals, pMesh->nNumVertices, 2);
		}
		else
		{
			VerticeBuffer[2] = GetDynamicRHI()->CreateVertexBuffer((float*)pMesh->pTextureCoords, pMesh->nNumVertices, 2);
		}
		AtrributeCount = 3;
		if (pMesh->pTangents)
		{
			VerticeBuffer[3] = GetDynamicRHI()->CreateVertexBuffer((float*)pMesh->pTangents, pMesh->nNumVertices, 4);	
		}
		else
		{
			VerticeBuffer[3] = GetDynamicRHI()->CreateVertexBuffer((float*)pMesh->pVertices, pMesh->nNumVertices, 4);
		}
		++AtrributeCount;

		if (pMesh->pBoneIDs)
		{
			int nSize = pMesh->nNumVertices * 4;
			float* pBoneId = new float[nSize];
			uint16* pSrcID = (uint16*)pMesh->pBoneIDs->BoneIDs;
			for (int i = 0; i < nSize; i++)
			{
				pBoneId[i] = (float)pSrcID[i];
			}
			VerticeBuffer[4] = GetDynamicRHI()->CreateVertexBuffer(pBoneId, pMesh->nNumVertices, 4);
			SAFE_DELETE_ARRAY(pBoneId);

			VerticeBuffer[5] = GetDynamicRHI()->CreateVertexBuffer((float*)pMesh->pBoneWeights->BoneWeights, pMesh->nNumVertices, 4);
			AtrributeCount += 2;
		}
	}
	else
	{
		auto& Buffer = VerticeBuffer[0];
		if (pMesh->pBoneIDs == NULL)
		{

			Buffer = GetDynamicRHI()->CreateVertexBuffer(4);
		}
		else
		{
			Buffer = GetDynamicRHI()->CreateVertexBuffer(6);
		}

		Buffer->CreateVertexBuffer((float*)pMesh->pVertices, pMesh->nNumVertices, 3, 0);
		Buffer->CreateVertexBuffer((float*)pMesh->pNormals, pMesh->nNumVertices, 3, 1);
		if (pMesh->pTextureCoords == NULL)
		{
			Buffer->CreateVertexBuffer((float*)pMesh->pVertices, pMesh->nNumVertices, 2, 2);
		}
		else
		{
			Buffer->CreateVertexBuffer((float*)pMesh->pTextureCoords, pMesh->nNumVertices, 2, 2);
		}

		if (pMesh->pTangents != NULL)
		{
			Buffer->CreateVertexBuffer((float*)pMesh->pTangents, pMesh->nNumVertices, 4, 3);
		}
		else
		{
			Buffer->CreateVertexBuffer((float*)pMesh->pVertices, pMesh->nNumVertices, 4, 3);
		}

		if (pMesh->pBoneIDs != NULL)
		{
			Buffer->CreateVertexBuffer(pMesh->pBoneIDs->BoneIDs, pMesh->nNumVertices, 4, 4);
			Buffer->CreateVertexBuffer((float*)pMesh->pBoneWeights, pMesh->nNumVertices, 4, 5);
		}
	}

	if (pMesh->pFacesIndex)
	{
		IndexBuffer = GetDynamicRHI()->CreateIndexBuffer(pMesh->pFacesIndex, pMesh->nNumFaces);
	}
	else
	{
		IndexBuffer = GetDynamicRHI()->CreateIndexBuffer(pMesh->pFacesIndex32, pMesh->nNumFaces);
	}
}

void CC3DMeshBuffer::UpdateVert(Vector3 * pVert,int nVert)
{
	if (VerticeBuffer[0])
	{
		VerticeBuffer[0]->UpdateVertexBUffer((float*)pVert, nVert, 3 * sizeof(float), 3 * sizeof(float));
	}

}
