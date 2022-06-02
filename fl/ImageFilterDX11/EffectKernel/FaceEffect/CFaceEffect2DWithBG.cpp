#include "CFaceEffect2DWithBG.h"
#include "BaseDefine/Define.h"
#include "BaseDefine/Vectors.h"
#include "EffectKernel/ShaderProgramManager.h"
//#include "opencv2/opencv.hpp"



CFaceEffect2DWithBG::CFaceEffect2DWithBG()
{
	m_isFirstInit = true;

	for (int i=0;i<MAX_SUPPORT_PEOPLE;i++)
	{
		m_VerticeBuffer[i] = NULL;
	}
	m_IndexBuffer = NULL;
	m_pConstantBuffer = NULL;
	m_2DInterFace = NULL;
	m_pMergeVertex = NULL;
	m_material = NULL;
	m_alpha = 1.0;
}


CFaceEffect2DWithBG::~CFaceEffect2DWithBG()
{
	Release();
}

void CFaceEffect2DWithBG::Release()
{
	SAFE_DELETE(m_2DInterFace);
	SAFE_RELEASE_BUFFER(m_pConstantBuffer);
	SAFE_RELEASE_BUFFER(m_IndexBuffer);
	for (int i = 0; i < MAX_SUPPORT_PEOPLE; i++)
	{
		SAFE_RELEASE_BUFFER(m_VerticeBuffer[i]);
	}

	SAFE_DELETE_ARRAY(m_pMergeVertex);
	SAFE_DELETE(m_material);
}

void * CFaceEffect2DWithBG::Clone()
{
	CFaceEffect2DWithBG* result = new CFaceEffect2DWithBG();
	*result = *this;
	return result;
}

bool CFaceEffect2DWithBG::ReadConfig(XMLNode & childNode, HZIP hZip ,char *pFilePath)
{
	if (!childNode.isEmpty())
	{
		XMLNode nodeDrawable = childNode.getChildNode("drawable", 0);
		if (!nodeDrawable.isEmpty())
		{
			const char *szDrawableName = nodeDrawable.getAttribute("Material");
			if (szDrawableName != NULL)
			{
				m_material = DXUtils::CreateTexFromZIP(hZip, szDrawableName, false);
			}
		}
		return true;
	}
	return false;
}

bool CFaceEffect2DWithBG::Prepare()
{
	//����shader
	CCVetexAttribute pAttribute[] =
	{
		{VERTEX_ATTRIB_POSITION, FLOAT_C3},
		{VERTEX_ATTRIB_TEX_COORD, FLOAT_C2}
	};
	string path = m_resourcePath + "/Shader/face2dMakeUp.fx";
	m_pShader = ShaderProgramManager::GetInstance()->GetOrCreateShaderByPathAndAttribs(path, pAttribute, 2);

	if (m_pConstantBuffer == NULL)
	{
		m_pConstantBuffer = DXUtils::CreateConstantBuffer(sizeof(float) * 4);
	}
	
	m_2DInterFace = new mt3dface::MTFace2DInterFace();
	return true;
}

void CFaceEffect2DWithBG::Render(BaseRenderParam &RenderParam)
{
	if (m_alpha < 0.001f)
	{
		return;
	}


	if (RenderParam.GetFaceCount()==0)
	{
		return;
	}

	auto pDoubleBuffer = RenderParam.GetDoubleBuffer();
	pDoubleBuffer->BindFBOA();
	m_pShader->useShader();
	auto pMaterialView = m_material->getTexShaderView();
	DeviceContextPtr->PSSetShaderResources(0, 1, &pMaterialView);
	DeviceContextPtr->PSSetSamplers(0, 1, &m_pSamplerLinear);

	float blendFactor[] = { 0.f,0.f,0.f,0.f };
	DeviceContextPtr->OMSetBlendState(m_pBlendStateNormal, blendFactor, 0xffffffff);


	float pParam[4];
	pParam[0] = m_alpha;
	DeviceContextPtr->UpdateSubresource(m_pConstantBuffer, 0, NULL, pParam, 0, 0);
	DeviceContextPtr->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
	DeviceContextPtr->PSSetConstantBuffers(0, 1, &m_pConstantBuffer);



	int nFaceCount = RenderParam.GetFaceCount();
	for (int faceIndex = 0; faceIndex < nFaceCount; faceIndex++)
	{
		Vector2 *pFacePoint = RenderParam.GetFacePoint(faceIndex,FACE_POINT_106);
		auto FaceMesh = m_2DInterFace->Get2DMesh((float*)pFacePoint, RenderParam.GetWidth(), RenderParam.GetHeight(), 0, mt3dface::MT_FACE_2D_BACKGROUND);
		MergeVertex(FaceMesh->pVertexs, FaceMesh->pTextureCoordinates, FaceMesh->nVertex);

		if (m_IndexBuffer == NULL)
		{
			m_IndexBuffer = DXUtils::CreateIndexBuffer(FaceMesh->pTriangleIndex, FaceMesh->nTriangle);
		}
		if (m_VerticeBuffer[faceIndex]==NULL)
		{
			m_VerticeBuffer[faceIndex] = DXUtils::CreateVertexBuffer(m_pMergeVertex, FaceMesh->nVertex, 5);
		}
		else
		{
			DXUtils::UpdateVertexBuffer(m_VerticeBuffer[faceIndex], m_pMergeVertex, FaceMesh->nVertex, 5 * sizeof(float), 5 * sizeof(float));
		}
		unsigned int nStride = (3 + 2) * sizeof(float);
		unsigned int nOffset = 0;
		DeviceContextPtr->IASetVertexBuffers(0, 1, &m_VerticeBuffer[faceIndex], &nStride, &nOffset);
		DeviceContextPtr->IASetIndexBuffer(m_IndexBuffer, DXGI_FORMAT_R16_UINT, 0);
		DeviceContextPtr->DrawIndexed(FaceMesh->nTriangle * 3, 0, 0);
	}

// 	{
// 		cv::Mat mImage = cv::Mat(pDoubleBuffer->GetFBOTextureB()->height(), pDoubleBuffer->GetFBOTextureB()->width(), CV_8UC4);
// 		pDoubleBuffer->GetFBOTextureB()->ReadTextureToCpu(mImage.data);
// 		cv::cvtColor(mImage, mImage, CV_RGBA2BGRA);
// 		cv::imwrite("resultB.png", mImage);
// 	}
}

void CFaceEffect2DWithBG::MergeVertex(float * pVertex, float * pUV, int nVertex)
{
	if (m_pMergeVertex == NULL)
	{
		m_pMergeVertex = new float[nVertex*(3 + 2)];
	}
	for (int i = 0;i<nVertex;i++)
	{
		m_pMergeVertex[i * 5] = pVertex[i*3];
		m_pMergeVertex[i * 5 + 1] = pVertex[i * 3 + 1];
		m_pMergeVertex[i * 5 + 2] = pVertex[i * 3 + 2];
		m_pMergeVertex[i * 5 + 3] = pUV[i * 2];
		m_pMergeVertex[i * 5 + 4] = pUV[i * 2 + 1];
	}
}
