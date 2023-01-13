#include "CFaceMeshChange.h"
#include "Toolbox/DXUtils/DXUtils.h"
#include "BaseDefine/Define.h"
#include <algorithm>
#include <iostream>
#include "Algorithm/MathUtils.h"
#include "EffectKernel/ShaderProgramManager.h"
#include "Toolbox/DXUtils/DX11Resource.h"
#include "Toolbox/Render/DynamicRHI.h"

CFaceMeshChange::CFaceMeshChange()
{

	for (int i = 0; i < MAX_SUPPORT_PEOPLE; i++)
	{
		m_VerticeBuffer[i] = NULL;
		m_rectVerticeBuffer[i] = NULL;
		m_rectVerticeBuffer2[i] = NULL;
	}
	m_nWidth = 0;
	m_nHeight = 0;
	m_pMergeVertex = NULL;
	m_2DInterFace = NULL;
	m_IndexBuffer = NULL;
	m_2DAnimation[0] = NULL;
	m_2DAnimation[1] = NULL;

	m_rectIndexBuffer = NULL;

	m_pShaderMat = NULL;
	m_nKeyFrame = 0;
	m_nAllFrame = 0;
	m_nFPS = 0;

}


CFaceMeshChange::~CFaceMeshChange()
{
	Release();
}

void CFaceMeshChange::Release()
{
	SAFE_RELEASE_BUFFER(m_IndexBuffer);
	SAFE_DELETE(m_2DInterFace);
	for (int i = 0; i < MAX_SUPPORT_PEOPLE; i++)
	{
		SAFE_RELEASE_BUFFER(m_VerticeBuffer[i]);
		SAFE_RELEASE_BUFFER(m_rectVerticeBuffer[i]);
		SAFE_RELEASE_BUFFER(m_rectVerticeBuffer2[i]);
	}
	SAFE_DELETE_ARRAY(m_pMergeVertex);
	SAFE_RELEASE_BUFFER(m_rectIndexBuffer);
	SAFE_DELETE(m_2DAnimation[0]);
	SAFE_DELETE(m_2DAnimation[1]);

}

void CFaceMeshChange::Resize(int nWidth, int nHeight)
{
	m_nWidth = nWidth;
	m_nHeight = nHeight;

}

void CFaceMeshChange::RenderMaterial(int nIndex,Vector2 *pFacePoint)
{
	static float standFaceWidth = 606.0/1500;
	static float standFaceHeight = 470.0/1500;
	Vector2 pStandLeftPoint(471.0/1500, 842.0/1500);
	Vector2 pStandRightPoint(1025.0/1500, 829.0/1500);

	Vector2 m_VTLeft[4] = {
		Vector2(0,0),Vector2(0,1),Vector2(0,0),Vector2(0,1),
	};
	m_VTLeft[2].x =  pStandLeftPoint.x + 0.1;
	m_VTLeft[3].x = pStandLeftPoint.x + 0.1;


	Vector2 m_VTRight[4] = {
	Vector2(1,0),Vector2(1,1),Vector2(1,0),Vector2(1,1),
	};
	m_VTRight[2].x = pStandRightPoint.x - 0.1;
	m_VTRight[3].x = pStandRightPoint.x -0.1 ;


	
	Vector2 V1, V2;
	V1.x = 612;
	V1.y = -9.2;
	V2 = pFacePoint[32] - pFacePoint[0];
	float fNormV1 = V1.length();
	float fNormV2 = V2.length();

	float tmp = (V1.x * V2.x + V1.y * V2.y) / (fNormV1*fNormV2);
	tmp = (std::min)(1.0f, tmp);
	tmp = (std::max)(-1.0f, tmp);
	float angle = std::acos(tmp);

	float fCrossVal = V1[0] * V2[1] - V2[0] * V1[1];
	if (fCrossVal < 0) angle = -angle;

	Matrix2   pRotate;
	pRotate[0] = std::cos(angle);
	pRotate[1] = -std::sin(angle);
	pRotate[2] = std::sin(angle);
	pRotate[3] = std::cos(angle);



	float faceWdith = fNormV2, faceHeight = (pFacePoint[71] - pFacePoint[16]).length();
	Vector2 pScale(faceWdith / (standFaceWidth*m_nWidth), faceHeight / (standFaceHeight*m_nHeight));
	Vector2 pVertLeft[8], pVertRight[8];

	//pScale.x *= m_nWidth / m_nHeight;
	for (int i=0;i<4;i++)
	{
		Vector2 Dist = pRotate * (m_VTLeft[i] - pStandLeftPoint)*Vector2(m_nWidth, m_nWidth)*pScale.x;
		pVertLeft[i * 2] = (Dist + pFacePoint[5])* Vector2(1.0 / m_nWidth, 1.0 / m_nHeight);

		Dist = pRotate * (m_VTRight[i] - pStandRightPoint)*Vector2(m_nWidth, m_nWidth)*pScale.x;
		pVertRight[i * 2] = (Dist + pFacePoint[27])* Vector2(1.0 / m_nWidth, 1.0 / m_nHeight);

 		pVertLeft[i*2] = pVertLeft[i*2] * 2.0 - Vector2(1.0, 1.0);
 		pVertRight[i*2] = pVertRight[i*2] * 2.0 - Vector2(1.0, 1.0);

		pVertLeft[i * 2 + 1] = m_VTLeft[i];
		pVertRight[i * 2 + 1] = m_VTRight[i];
	}


	if (m_rectIndexBuffer == NULL)
	{
		unsigned short index[] =
		{
			0, 1, 2,
			1, 2, 3
		};
		m_rectIndexBuffer = DXUtils::CreateIndexBuffer(index, 2);
	}
	if (m_rectVerticeBuffer[nIndex] == NULL)
	{
		m_rectVerticeBuffer[nIndex] = DXUtils::CreateVertexBuffer((float*)pVertLeft, 4, m_nVertsSize);
	}
	else
	{
		DXUtils::UpdateVertexBuffer(m_rectVerticeBuffer[nIndex], (float*)pVertLeft, 4, m_nVertsSize * sizeof(float), m_nVertsSize * sizeof(float));
	}

	if (m_rectVerticeBuffer2[nIndex] == NULL)
	{
		m_rectVerticeBuffer2[nIndex] = DXUtils::CreateVertexBuffer((float*)pVertRight, 4, m_nVertsSize);
	}
	else
	{
		DXUtils::UpdateVertexBuffer(m_rectVerticeBuffer2[nIndex], (float*)pVertRight, 4, m_nVertsSize * sizeof(float), m_nVertsSize * sizeof(float));
	}


	

	m_pShaderMat->useShader();



	float blendFactor[] = { 0.f,0.f,0.f,0.f };
	DeviceContextPtr->OMSetBlendState(m_pBlendStateNormal, blendFactor, 0xffffffff);
	//设置顶点数据
	unsigned int nStride = 4 * sizeof(float);
	unsigned int nOffset = 0;

// 	auto pDoubleBuffer = RenderParam.GetDoubleBuffer();
// 
// 	pDoubleBuffer->BindFBOA();

	//auto pMaterialView = m_2DAnimation[0]->GetSRV(m_runTime);
	//DeviceContextPtr->PSSetShaderResources(0, 1, &pMaterialView);
	auto pMaterialView = m_2DAnimation[0]->GetTex(m_runTime);
	pMaterialView->Bind(0);
	DeviceContextPtr->PSSetSamplers(0, 1, &m_pSamplerLinear);

	DeviceContextPtr->IASetVertexBuffers(0, 1, &m_rectVerticeBuffer[nIndex], &nStride, &nOffset);
	DeviceContextPtr->IASetIndexBuffer(m_rectIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
	DeviceContextPtr->DrawIndexed(2 * 3, 0, 0);
// 	pDoubleBuffer->SyncAToB();
// 	pDoubleBuffer->BindFBOB();

	//auto pMaterialView2 = m_2DAnimation[1]->GetSRV(m_runTime);
	//DeviceContextPtr->PSSetShaderResources(0, 1, &pMaterialView2);
	auto pMaterialView2 = m_2DAnimation[1]->GetTex(m_runTime);
	pMaterialView2->Bind(0);
	DeviceContextPtr->PSSetSamplers(0, 1, &m_pSamplerLinear);
	DeviceContextPtr->IASetVertexBuffers(0, 1, &m_rectVerticeBuffer2[nIndex], &nStride, &nOffset);
	DeviceContextPtr->IASetIndexBuffer(m_rectIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
	DeviceContextPtr->DrawIndexed(2 * 3, 0, 0);

//	pDoubleBuffer->SwapFBO();
}


void * CFaceMeshChange::Clone()
{
	CFaceMeshChange* result = new CFaceMeshChange();
	*result = *this;
	return result;
}

bool CFaceMeshChange::ReadConfig(XMLNode & childNode, HZIP hZip, char * pFilePath)
{
	if (!childNode.isEmpty())
	{
		for (int i=0;i<2;i++)
		{
			XMLNode nodeDrawable = childNode.getChildNode("anidrawable", i);
			if (!nodeDrawable.isEmpty()) {
				const char* szDrawableName = nodeDrawable.getAttribute("name");
				AnimationDrawable* drawable = new AnimationDrawable();

				const char* szOffset = nodeDrawable.getAttribute("offset");
				long offset = atol(szOffset);
				drawable->setOffset(offset);

				const char* szLoopMode = nodeDrawable.getAttribute("loopMode");
				if (szLoopMode != NULL && strcmp(szLoopMode, "oneShot") == 0)
					drawable->setLoopMode(ELM_ONESHOT);
				else
					drawable->setLoopMode(ELM_REPEAT);

				const char* szGenMipmap = nodeDrawable.getAttribute("genMipmaps");
				bool bGenMipmap = false;
				if (szGenMipmap != NULL && (strcmp(szGenMipmap, "yes") || strcmp(szGenMipmap, "YES")))
					bGenMipmap = true;

				const char *szItems = nodeDrawable.getAttribute("items");
				const char *szItemInfo = nodeDrawable.getAttribute("iteminfo");
				if (szItems != NULL && szItemInfo != NULL)
				{
					int iStart = 0;
					int nCount = 0;
					int nDuring = 0;
					int nStep = 1;
					char szImagePath[128];
					sscanf(szItemInfo, "%d,%d,%d,%d", &iStart, &nCount, &nDuring, &nStep);
					m_nFPS = nDuring/2;
					if (nCount > 0 && nDuring > 0)
					{
						for (; iStart <= nCount; iStart += nStep)
						{
							sprintf(szImagePath, szItems, iStart);

							std::shared_ptr< MaterialTexRHI> TexRHI = GetDynamicRHI()->CreateAsynTextureZIP(hZip, szImagePath, bGenMipmap);

							long during = nDuring;

							drawable->appandTex(during, TexRHI);
						}
					}
				}

				m_2DAnimation[i] = drawable;
			}
		}

		XMLNode nodeModel = childNode.getChildNode("animate", 0);
		const char *szFrame = nodeModel.getAttribute("frame");
		const char *szkeyFrame = nodeModel.getAttribute("keyframe");
		m_nAllFrame = atoi(szFrame);
		m_nKeyFrame = atoi(szkeyFrame);
		if (m_nAllFrame >0&&m_nKeyFrame>0)
		{
			int j = -1;
			m_keyValue.resize(m_nKeyFrame);
			XMLNode nodeVertex = nodeModel.getChildNode("keyValue", ++j);
			while (!nodeVertex.isEmpty())
			{
				const char *szPos = nodeVertex.getAttribute("keyIndex");
				sscanf(szPos, "%d,%f", &m_keyValue[j].key, &m_keyValue[j].alpha);
				nodeVertex = nodeModel.getChildNode("keyValue", ++j);
			}

		}
		return true;
	}
	return false;
}

bool CFaceMeshChange::Prepare()
{
	CCVetexAttribute pAttribute[] =
	{
		{VERTEX_ATTRIB_POSITION, FLOAT_C2},
		{VERTEX_ATTRIB_TEX_COORD, FLOAT_C2}
	};
	string path = m_resourcePath + "/Shader/facemeshChange.fx";
	m_pShader = ShaderProgramManager::GetInstance()->GetOrCreateShaderByPathAndAttribs(path, pAttribute, 2);

	path = m_resourcePath + "/Shader/face2dTexture.fx";
	m_pShaderMat = ShaderProgramManager::GetInstance()->GetOrCreateShaderByPathAndAttribs(path, pAttribute, 2);

	m_2DInterFace = new mt3dface::MTFace2DInterFace();
	m_nVertsSize = 4;


	return true;
}



void CFaceMeshChange::Render(BaseRenderParam & RenderParam)
{

	if (RenderParam.GetFaceCount() == 0)
	{
		return;
	}
//	DXUtils::SetRasterizerState(false);
	Resize(RenderParam.GetWidth(), RenderParam.GetHeight());

	auto pDoubleBuffer = RenderParam.GetDoubleBuffer();
	int nFaceCount = RenderParam.GetFaceCount();
	for (int faceIndex = 0; faceIndex < nFaceCount; faceIndex++)
	{
		//pDoubleBuffer->SyncAToB();
		m_pShader->useShader();
		pDoubleBuffer->BindFBOA();


		DeviceContextPtr->PSSetSamplers(0, 1, &m_pSamplerLinear);

		Vector2 *pSrcpoint118 = RenderParam.GetFacePoint(faceIndex, FACE_POINT_130);
 		Vector2 pDstPoint[118];
 		memcpy(pDstPoint, pSrcpoint118, 118 * sizeof(Vector2));
 		CalFaceChangePoint(pSrcpoint118, pDstPoint);

		auto FaceMesh = m_2DInterFace->Get2DMesh((float*)pSrcpoint118, RenderParam.GetWidth(), RenderParam.GetHeight(), 0, mt3dface::MT_FACE_25D);
		Vector2 pSrcpoint171[442];
		Vector2 pDstPoint171[442];
		for (int i =0;i<FaceMesh->nVertex;i++)
		{
			pSrcpoint171[i].x = FaceMesh->pVertexs[i * 3];
			pSrcpoint171[i].y = FaceMesh->pVertexs[i * 3+1];
		}
		m_2DInterFace->Get2DMesh((float*)pDstPoint, RenderParam.GetWidth(), RenderParam.GetHeight(), 0, mt3dface::MT_FACE_25D);
		for (int i = 0; i < FaceMesh->nVertex; i++)
		{
			pDstPoint171[i].x = FaceMesh->pVertexs[i * 3];
			pDstPoint171[i].y = FaceMesh->pVertexs[i * 3 + 1];
		}

		memcpy(pDstPoint171 + 86, pSrcpoint171 + 86, 18 * sizeof(Vector2));
		memcpy(pDstPoint171 + 186, pSrcpoint171 + 186, (FaceMesh->nVertex-186) * sizeof(Vector2));
		MergeVertex((float*)pDstPoint171, (float*)pSrcpoint171, FaceMesh->nVertex);

		pDoubleBuffer->SyncAToBRegion((float*)pDstPoint171, FaceMesh->nVertex, 2, 1);
		//auto pSrcShaderView = pDoubleBuffer->GetFBOTextureB()->getTexShaderView();
		//DeviceContextPtr->PSSetShaderResources(0, 1, &pSrcShaderView);
		GetDynamicRHI()->SetPSShaderResource(0, RHIResourceCast(pDoubleBuffer.get())->GetFBOTextureB());

		if (m_IndexBuffer == NULL)
		{
			m_IndexBuffer = DXUtils::CreateIndexBuffer(FaceMesh->pTriangleIndex, FaceMesh->nTriangle);
		}
		if (m_VerticeBuffer[faceIndex] == NULL)
		{
			m_VerticeBuffer[faceIndex] = DXUtils::CreateVertexBuffer(m_pMergeVertex, FaceMesh->nVertex, m_nVertsSize);
		}
		else
		{
			DXUtils::UpdateVertexBuffer(m_VerticeBuffer[faceIndex], m_pMergeVertex, FaceMesh->nVertex, m_nVertsSize * sizeof(float), m_nVertsSize * sizeof(float));
		}
		unsigned int nStride = m_nVertsSize * sizeof(float);
		unsigned int nOffset = 0;
		DeviceContextPtr->IASetVertexBuffers(0, 1, &m_VerticeBuffer[faceIndex], &nStride, &nOffset);
		DeviceContextPtr->IASetIndexBuffer(m_IndexBuffer, DXGI_FORMAT_R16_UINT, 0);
		DeviceContextPtr->DrawIndexed(FaceMesh->nTriangle*3, 0, 0);
		RenderMaterial(faceIndex,pSrcpoint118);
	//	DeviceContextPtr->Flush();
	//	pDoubleBuffer->SwapFBO();
		
	}

//	DXUtils::SetRasterizerState(true);
} 


void CFaceMeshChange::CalFaceChangePoint(Vector2 * pSrcPoint118, Vector2 * pDstPoint118)
{
// 	float m_alpha[] =
// 	{
// 		0.0,0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9,1.0,0.5,0.0,0.2,0.0,0.1,0.0,0.0
// 	};
// 	int sumframe = 1500;
// 	int index = m_runTime % sumframe;
// 	float fIndex = index * 17.0f / sumframe;
// 
// 	index = fIndex;
// 	fIndex = fIndex - index;
// 	float alpha = m_alpha[index] * (1 - fIndex) + m_alpha[index + 1] *  fIndex;
// 

	int nfps = m_nFPS;
	int sumtime = m_nAllFrame * nfps;
	int KeyIndex = m_runTime % sumtime;
	float findex = KeyIndex *1.0f/ nfps;

	float alpha = 0;
	for (int i=1;i<m_nKeyFrame;i++)
	{
		if (m_keyValue[i].key>findex)
		{
			float tmp = (m_keyValue[i].key - findex) / (m_keyValue[i].key - m_keyValue[i - 1].key);
			alpha = m_keyValue[i - 1].alpha*tmp + m_keyValue[i].alpha*(1.0 - tmp);
			break;
		}
	}
	
// 	if (partMLS !=NULL)
// 	{
// 		partMLS->m_alpha = 1.0 - alpha;
// 	}

	pDstPoint118[5] = pSrcPoint118[5] + (pSrcPoint118[0] - pSrcPoint118[32])*0.4*alpha;

	std::vector<Vector2> InPoint;
	std::vector<Vector2> OuPoint;
	InPoint.push_back(pDstPoint118[1]);
	InPoint.push_back(pDstPoint118[5]);
	InPoint.push_back(pDstPoint118[9]);

	CMathUtils::LagrangePolyfit(InPoint, OuPoint, 9);

	for (int i = 1; i < 8; i++)
	{
		pDstPoint118[i + 1] = OuPoint[i];
	}


	pDstPoint118[27] = pSrcPoint118[27] + (pSrcPoint118[32] - pSrcPoint118[0])*0.4*alpha;;

	InPoint.clear();
	OuPoint.clear();
	InPoint.push_back(pDstPoint118[31]);
	InPoint.push_back(pDstPoint118[27]);
	InPoint.push_back(pDstPoint118[23]);

	CMathUtils::LagrangePolyfit(InPoint, OuPoint, 9);

	for (int i = 1; i < 8; i++)
	{
		pDstPoint118[31 - i] = OuPoint[i];
	}


}

void CFaceMeshChange::MergeVertex(float * pVertex,   float *pMaskUV, int nVertex)
{
	if (m_pMergeVertex == NULL)
	{
		m_pMergeVertex = new float[nVertex*m_nVertsSize];
	}
	for (int i = 0; i < nVertex; i++)
	{
		m_pMergeVertex[i * m_nVertsSize] = pVertex[i * 2];
		m_pMergeVertex[i * m_nVertsSize + 1] = pVertex[i * 2 + 1];
		m_pMergeVertex[i * m_nVertsSize + 2] = pMaskUV[i * 2] * 0.5 + 0.5;
		m_pMergeVertex[i * m_nVertsSize + 3] = pMaskUV[i * 2 + 1] * 0.5 + 0.5;
	}

// 	freopen("point.txt", "w", stdout);
//  	std::cout << nVertex << std::endl;
//  	for (int i = 0; i < nVertex; i++) {
//  		std::cout << pVertex[i * 2]*0.5+0.5<< "," << pVertex[i * 2+1]*0.5+0.5 << ",";
//  	}
// 	fclose(stdout);


}