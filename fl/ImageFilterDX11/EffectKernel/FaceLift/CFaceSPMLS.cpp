#include "CFaceSPMLS.h"
#include "Toolbox/DXUtils/DXUtils.h"
#include "BaseDefine/Define.h"
#include <algorithm>
#include <iostream>
#include "EffectKernel/ShaderProgramManager.h"
#include "Toolbox/DXUtils/DX11Resource.h"
#include "Toolbox/Render/DynamicRHI.h"

CFaceSPMLS::CFaceSPMLS()
{
	m_EffectPart = FACE_EFFECT_OFFSETSHAPE;
	m_SmallTextureWidth = 80;
	m_SmallTextureHeight = 80;
	m_nBigSize = 0;
	m_pShaderFilterBig = NULL;
	m_pBigOffestFBO = NULL;
	m_pBigoffestTexture = NULL;
	m_pOffestTexture = NULL;
	m_IndexBuffer = NULL;
	m_VerticeBuffer2 = NULL;
	m_IndexBuffer2 = NULL;
	for (int i = 0; i < MAX_SUPPORT_PEOPLE; i++)
	{
		m_VerticeBuffer[i] = NULL;
	}
	m_nWidth = 0;
	m_nHeight = 0;
	m_VTPointCount = 0;
	m_InterFM = NULL;
	m_pMergeVertex = NULL;
	m_pConstantBuffer = NULL;

	m_FaceShape = NULL;
	m_EllipseShape = NULL;
}


CFaceSPMLS::~CFaceSPMLS()
{
	Release();
}

void CFaceSPMLS::Release()
{
	SAFE_DELETE(m_pBigOffestFBO);
	SAFE_DELETE(m_pBigoffestTexture);
	SAFE_DELETE_ARRAY(m_pOffestTexture);
	SAFE_RELEASE_BUFFER(m_IndexBuffer);
	SAFE_RELEASE_BUFFER(m_IndexBuffer2);
	for (int i = 0; i < MAX_SUPPORT_PEOPLE; i++)
	{
		SAFE_RELEASE_BUFFER(m_VerticeBuffer[i]);
	}
	SAFE_RELEASE_BUFFER(m_VerticeBuffer2);
	SAFE_RELEASE_BUFFER(m_pConstantBuffer);
	SAFE_DELETE(m_InterFM);
	SAFE_DELETE_ARRAY(m_pMergeVertex);

	if (m_hasCopy)
	{
		m_FaceShape = NULL;
		m_EllipseShape = NULL;
	}
	else
	{
		SAFE_DELETE(m_FaceShape);
		SAFE_DELETE(m_EllipseShape);
	}


}

void CFaceSPMLS::Resize(int nWidth, int nHeight)
{
	m_nWidth = nWidth;
	m_nHeight = nHeight;

	int nBigSize = (std::min)(m_nWidth, m_nHeight);
	if (nBigSize != m_nBigSize)
	{
		m_nBigSize = nBigSize;
		FilterToBigFBO();
	}

}


void CFaceSPMLS::GenaratePoint(std::vector<Vector2> &point, std::vector<unsigned short> &vIndex, std::vector<Vector4> &Offest)
{
	int width =  m_SmallTextureWidth;
	int height =  m_SmallTextureHeight;
	float stepx = 1.0 / width;
	float stepy = 1.0 / height;

	for (int i=0;i<=height;i++)
	{
		for (int j =0;j<=width;j++)
		{
			point.push_back(Vector2(j*stepx, i*stepy));
		}
	}
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			int Index1 = i * (width + 1) + j;
			int Index2 = Index1 + 1;
			int Index3 = Index1 + width + 1;
			int Index4 = Index3 + 1;
			vIndex.push_back(Index1);
			vIndex.push_back(Index2);
			vIndex.push_back(Index3);
			vIndex.push_back(Index2);
			vIndex.push_back(Index3);
			vIndex.push_back(Index4);
		}
	}

	int nSize = point.size();
	Offest.resize(nSize);
	for (int j = 0; j < nSize; ++j) {
		int x = point[j].x*m_SmallTextureWidth + 0.1;
		int y = point[j].y*m_SmallTextureHeight + 0.1;
		x = (std::max)(0, (std::min)(x, m_SmallTextureWidth - 1));
		y = (std::max)(0, (std::min)(y, m_SmallTextureHeight - 1));
		int pos = (y*m_SmallTextureWidth + x) *2;

		float xc = m_pOffestTexture[pos + 0] +127;

		float yc = m_pOffestTexture[pos + 1] +127;

		Vector4 offestPos(xc, yc, 0.0,255.0);
		offestPos = offestPos * 1.0f / 255;
		Offest[j] = offestPos;
	}



}


void CFaceSPMLS::FilterToBigFBO()
{
	SAFE_DELETE(m_pBigOffestFBO);
	SAFE_DELETE(m_pBigoffestTexture);
	m_pBigoffestTexture = new DX11Texture();
	m_pBigoffestTexture->initTexture(DXGI_FORMAT_R32G32B32A32_FLOAT, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET, m_nBigSize, m_nBigSize, NULL, 0, false, false);
	m_pBigOffestFBO = new DX11FBO();
	m_pBigOffestFBO->initWithTexture(m_nBigSize, m_nBigSize, false, m_pBigoffestTexture->getTex());

	int nVertSize = 8;
	if (m_VerticeBuffer2 == NULL)
	{
		std::vector<Vector2> Vtpoint;
		std::vector<Vector4> Offset;
		std::vector<unsigned short>vIndex;
		GenaratePoint(Vtpoint, vIndex,Offset);
		m_VTPointCount = Vtpoint.size();
		float* pMergeVertex = new float[m_VTPointCount * nVertSize];
		for (int i = 0; i < m_VTPointCount; i++)
		{
			pMergeVertex[i * nVertSize] = Vtpoint[i].x;
			pMergeVertex[i * nVertSize + 1] = Vtpoint[i].y;
			pMergeVertex[i * nVertSize + 2] = 0.5;
			pMergeVertex[i * nVertSize + 3] = 1.0;
			pMergeVertex[i * nVertSize + 4] = Offset[i].x;
			pMergeVertex[i * nVertSize + 5] = Offset[i].y;
			pMergeVertex[i * nVertSize + 6] = Offset[i].z;
			pMergeVertex[i * nVertSize + 7] = Offset[i].w;
		}
		m_VerticeBuffer2 = DXUtils::CreateVertexBuffer(pMergeVertex, m_VTPointCount, nVertSize);
		m_IndexBuffer2 = DXUtils::CreateIndexBuffer(vIndex.data(), vIndex.size() / 3);
		m_VTTriangleSize = vIndex.size();
		SAFE_DELETE_ARRAY(pMergeVertex);

	}
	m_pShaderFilterBig->useShader();
	m_pBigOffestFBO->bind();
	m_pBigOffestFBO->clear(0, 0, 0, 0);
	unsigned int nStride = nVertSize * sizeof(float);
	unsigned int nOffset = 0;
	DeviceContextPtr->IASetVertexBuffers(0, 1, &m_VerticeBuffer2, &nStride, &nOffset);
	DeviceContextPtr->IASetIndexBuffer(m_IndexBuffer2, DXGI_FORMAT_R16_UINT, 0);
	DeviceContextPtr->DrawIndexed(m_VTTriangleSize, 0, 0);
}

void CFaceSPMLS::RunFace106To118(Vector2 * pFacePoint, Vector2 *pFacePoint118)
{
	memcpy(pFacePoint118, pFacePoint, 106 * sizeof(Vector2));
	pFacePoint118[106] = (pFacePoint[99] + pFacePoint[100])*0.5;
	pFacePoint118[107] = (pFacePoint[101] + pFacePoint[100])*0.5;
	pFacePoint118[108] = (pFacePoint[105] + pFacePoint[104])*0.5;
	pFacePoint118[109] = (pFacePoint[104] + pFacePoint[103])*0.5;

	pFacePoint118[110] = (pFacePoint[86] + pFacePoint[87])*0.5;
	pFacePoint118[111] = (pFacePoint[87] + pFacePoint[88])*0.5;
	pFacePoint118[112] = (pFacePoint[90] + pFacePoint[91])*0.5;
	pFacePoint118[113] = (pFacePoint[92] + pFacePoint[91])*0.5;

	pFacePoint118[114] = (pFacePoint[52] + pFacePoint[58])*0.5;
	pFacePoint118[115] = (pFacePoint[53] + pFacePoint[56])*0.5;
	pFacePoint118[116] = (pFacePoint[62] + pFacePoint[68])*0.5;
	pFacePoint118[117] = (pFacePoint[63] + pFacePoint[66])*0.5;
}

void * CFaceSPMLS::Clone()
{
	CFaceSPMLS* result = new CFaceSPMLS();
	*result = *this;
	return result;
}

bool CFaceSPMLS::ReadConfig(XMLNode & childNode, HZIP hZip, char *pFilePath)
{

	CEffectPart::ReadConfig(childNode, hZip, pFilePath);
	ReadConfig(childNode, hZip, pFilePath, "");
	return true;
}

bool CFaceSPMLS::ReadConfig(XMLNode& childNode, const std::string &path)
{
	CEffectPart::ReadConfig(childNode);
	ReadConfig(childNode, 0, NULL, path);
	return true;
}

bool CFaceSPMLS::Prepare()
{
	CCVetexAttribute pAttribute[] =
	{
		{VERTEX_ATTRIB_POSITION, FLOAT_C4},
		{VERTEX_ATTRIB_TEX_COORD, FLOAT_C2}
	};
	string path = m_resourcePath + "/Shader/faceMLSByImg.fx";
	m_pShader = ShaderProgramManager::GetInstance()->GetOrCreateShaderByPathAndAttribs(path, pAttribute, 2);

	CCVetexAttribute pAttribute2[] =
	{
		{VERTEX_ATTRIB_POSITION, FLOAT_C4},
	    {VERTEX_ATTRIB_COLOR,FLOAT_C4},
	};
	string path1 = m_resourcePath + "/Shader/faceMLSToBigImg.fx";
	m_pShaderFilterBig = ShaderProgramManager::GetInstance()->GetOrCreateShaderByPathAndAttribs(path1, pAttribute2, 2);

	m_InterFM = new InterFacePointDeform();
	m_nVertsSize = 6;
	return true;
}



void CFaceSPMLS::Render(BaseRenderParam & RenderParam)
{
	if (RenderParam.GetFaceCount() == 0)
	{
		return;
	}
	long runTime1 = GetRunTime();
	if (runTime1 < 0)
	{
		return;
	}
	//GetOffsetTexture();
//	DXUtils::SetRasterizerState(false);
	Resize(RenderParam.GetWidth(), RenderParam.GetHeight());

	auto pDoubleBuffer = RenderParam.GetDoubleBuffer();

	int nFaceCount = RenderParam.GetFaceCount();
	for (int faceIndex = 0; faceIndex < nFaceCount; faceIndex++)
	{

		Vector2 pSrcpoint118[118];
		RunFace106To118(RenderParam.GetFacePoint(faceIndex, FACE_POINT_106),pSrcpoint118);
		Vector2 pSrcpoint442[442];
		m_InterFM->RunFace118To442(pSrcpoint118, 0, pSrcpoint442);
		float pTransformMatrix[8];
		GetOffsetTransform(pSrcpoint118,m_nWidth, m_nHeight, pTransformMatrix);
		float sideParam = GetSideFaceParam((float*)pSrcpoint118, 0.5)*m_alpha;
		pTransformMatrix[0] *= sideParam;
		pTransformMatrix[1] *= sideParam;
		pTransformMatrix[2] *= sideParam;
		pTransformMatrix[3] *= sideParam;
		pTransformMatrix[4] = m_nWidth;
		pTransformMatrix[5] = m_nHeight;
		pTransformMatrix[6] = 1.0/m_nWidth;
		pTransformMatrix[7] = 1.0/m_nHeight;

		Vector2 pScaleValue442[442];
		GetFaceScaleEachPoint(pSrcpoint118, m_InterFM->m_StandStrPoint, m_InterFM->m_StandIndex, pScaleValue442);
		for (int i = 0; i < 442; i++) {
			pSrcpoint442[i].x = pSrcpoint442[i].x / m_nWidth;
			pSrcpoint442[i].y = pSrcpoint442[i].y / m_nHeight;
		}

		MergeVertex((float*)pSrcpoint442, (float*)pScaleValue442, (float*)m_InterFM->m_StandDstPoint, 442);
		if (m_IndexBuffer == NULL)
		{
			m_IndexBuffer = DXUtils::CreateIndexBuffer(m_InterFM->m_StandIndex, 832);
		}
		if (m_VerticeBuffer[faceIndex] == NULL)
		{
			m_VerticeBuffer[faceIndex] = DXUtils::CreateVertexBuffer(m_pMergeVertex, 442, m_nVertsSize);
		}
		else
		{
			DXUtils::UpdateVertexBuffer(m_VerticeBuffer[faceIndex], m_pMergeVertex, 442, m_nVertsSize * sizeof(float), m_nVertsSize * sizeof(float));
		}
		if (m_pConstantBuffer == NULL)
		{
			m_pConstantBuffer = DXUtils::CreateConstantBuffer(sizeof(float) * 8);
		}

	//	pDoubleBuffer->SyncAToB();
		m_pShader->useShader();
		pDoubleBuffer->BindFBOA();
		pDoubleBuffer->SyncAToBRegion((float*)pSrcpoint442, 442);
		auto pMaterialView = m_pBigoffestTexture->getTexShaderView();
		//auto pSrcShaderView = pDoubleBuffer->GetFBOTextureB()->getTexShaderView();
		//DeviceContextPtr->PSSetShaderResources(0, 1, &pSrcShaderView);
		GetDynamicRHI()->SetPSShaderResource(0, RHIResourceCast(pDoubleBuffer.get())->GetFBOTextureB());
		DeviceContextPtr->PSSetSamplers(0, 1, &m_pSamplerLinear);
		DeviceContextPtr->PSSetShaderResources(1, 1, &pMaterialView);
		DeviceContextPtr->UpdateSubresource(m_pConstantBuffer, 0, NULL, pTransformMatrix, 0, 0);
		DeviceContextPtr->PSSetConstantBuffers(0, 1, &m_pConstantBuffer);
		unsigned int nStride = m_nVertsSize * sizeof(float);
		unsigned int nOffset = 0;
		DeviceContextPtr->IASetVertexBuffers(0, 1, &m_VerticeBuffer[faceIndex], &nStride, &nOffset);
		DeviceContextPtr->IASetIndexBuffer(m_IndexBuffer, DXGI_FORMAT_R16_UINT, 0);
		DeviceContextPtr->DrawIndexed(832*3 , 0, 0);
	}

//	DXUtils::SetRasterizerState(true);
} 


float CFaceSPMLS::CalArea(Vector2 point1, Vector2 point2, Vector2 point3)
{
	float a = sqrt(pow((point1.x - point2.x), 2) + pow((point1.y - point2.y), 2));
	float b = sqrt(pow((point2.x - point3.x), 2) + pow((point2.y - point3.y), 2));
	float c = sqrt(pow((point3.x - point1.x), 2) + pow((point3.y - point1.y), 2));
	float p = (a + b + c) / 2;
	float s = 0.0f;
	if ((p - a)*(p - b)*(p - c) > 0)
	{
		s = sqrt(p*(p - a)*(p - b)*(p - c));
	}
	return s;
}

void CFaceSPMLS::GetOffsetTransform(Vector2 *pPoint118, int nImgWidth, int nImgHeight, float *pOutMat)
{
	float V1[2], V2[2];
	V1[0] = 1;
	V1[1] = 0;
	V2[0] = pPoint118[32].x - pPoint118[0].x;
	V2[1] = -(pPoint118[32].y - pPoint118[0].y);
	float fNormV1 = std::sqrt(V1[0] * V1[0] + V1[1] * V1[1]);
	float fNormV2 = std::sqrt(V2[0] * V2[0] + V2[1] * V2[1]);
	float tmp = (V1[0] * V2[0] + V1[1] * V2[1]) / (fNormV1*fNormV2);
	tmp = (std::min)(1.0f, tmp);
	tmp = (std::max)(-1.0f, tmp);
	float angle = -std::acos(tmp);
	float fCrossVal = V1[0] * V2[1] - V2[0] * V1[1];
	if (fCrossVal < 0) angle = -angle;

	pOutMat[0] = std::cos(angle)*(1500.0f / (float)nImgWidth);
	pOutMat[1] = -std::sin(angle)*(1500.0f / (float)nImgWidth);
	pOutMat[2] = std::sin(angle)*(1500.0f / (float)nImgHeight);
	pOutMat[3] = std::cos(angle)*(1500.0f / (float)nImgHeight);
}

void CFaceSPMLS::GetFaceScaleEachPoint(Vector2 * pPoint118, Vector2 * pStandard442Src, unsigned short *pFaceMesh, Vector2 * pScaleValue442)
{
	Vector2 pPoint442User[442];
	Vector2 pPoint442Standard[442];
	m_InterFM->RunFace118To442((Vector2*)pPoint118, 0, pPoint442User, false, true);
	//ԭʼ�㣨���������������������ù̶��㣩

	for (int i = 0; i < 442; i++)
	{
		pPoint442Standard[i].x = pStandard442Src[i].x * 1000.0f;
		pPoint442Standard[i].y = pStandard442Src[i].y * 1500.0f;

	}
	std::vector<float> vecPointArea[442];
	for (int i = 0; i < 832; i++)
	{
		Vector2 V1u = pPoint442User[pFaceMesh[i * 3]];
		Vector2 V2u = pPoint442User[pFaceMesh[i * 3 + 1]];
		Vector2 V3u = pPoint442User[pFaceMesh[i * 3 + 2]];

		Vector2 V1s = pPoint442Standard[pFaceMesh[i * 3]];
		Vector2 V2s = pPoint442Standard[pFaceMesh[i * 3 + 1]];
		Vector2 V3s = pPoint442Standard[pFaceMesh[i * 3 + 2]];
		float fAreaUse = CalArea(V1u, V2u, V3u);
		float fAreaStd = CalArea(V1s, V2s, V3s);
		if (fAreaStd > 0.1f)
		{
			float fRatio = std::sqrt(fAreaUse) / std::sqrt(fAreaStd);
			vecPointArea[pFaceMesh[i * 3]].push_back(fRatio);
			vecPointArea[pFaceMesh[i * 3 + 1]].push_back(fRatio);
			vecPointArea[pFaceMesh[i * 3 + 2]].push_back(fRatio);
		}
	}

	for (int i = 0; i < 442; i++)
	{
		float minRatio = 0.0f;
		for (int j = 0; j < vecPointArea[i].size(); j++)
			minRatio += vecPointArea[i][j];
		if (vecPointArea[i].size() == 0) {
			pScaleValue442[i] = Vector2(1.0, 1.0);
			continue;
		}
		minRatio /= (float)vecPointArea[i].size();
		float ScaleiX = minRatio;
		float ScaleiY = 1.0f;
		if (i == 41)
		{
			ScaleiY = 1.0f;
		}
		else
		{
			float DistNoseU = pPoint442User[i].distance(pPoint442User[41]);
			float DistNoseS = pPoint442Standard[i].distance(pPoint442Standard[41]);
			ScaleiY = DistNoseU / DistNoseS;
		}
		pScaleValue442[i] = Vector2(ScaleiX, ScaleiY);
	}

	pScaleValue442[41] = pScaleValue442[40];

	int indexLipUp[] = { 59,60,61,62,63,64,65 };
	int indexLipBottom[] = { 66,67,68,69,70,71,72 };
	int indexMouthUp[] = { 51,52,53,54,55,56,57 };
	int indexMouthBottom[] = { 73,74,75,76,77,78,79 };


	for (int i = 0; i < 7; i++)
	{
		pScaleValue442[indexLipUp[i]] = 0.5f*(pScaleValue442[indexMouthUp[i]] + pScaleValue442[indexMouthBottom[i]]);
		pScaleValue442[indexLipBottom[i]] = pScaleValue442[indexLipUp[i]];
	}
	pScaleValue442[50] = 0.5f*(pScaleValue442[51] + pScaleValue442[73]);
	pScaleValue442[58] = 0.5f*(pScaleValue442[57] + pScaleValue442[79]);

}


float CFaceSPMLS::GetSideFaceParam(float * pPoint118, float SideParam)
{
	Vector2 *Point118 = (Vector2*)pPoint118;

	float  leftDist = Point118[0].distance(Point118[71]);//CalDistance(pPoint118[0 * 2], pPoint118[0 * 2 + 1], pPoint118[71 * 2], pPoint118[71 * 2 + 1]);
	float rightDist = Point118[32].distance(Point118[71]);//CalDistance(pPoint118[32 * 2], pPoint118[32 * 2 + 1], pPoint118[71 * 2], pPoint118[71 * 2 + 1]);

	float minDist = (std::min)(leftDist, rightDist);
	float maxDist = (std::max)(leftDist, rightDist);

	float side = minDist / maxDist;

	return SideParam * (1 - side) + side;

}

void CFaceSPMLS::MergeVertex(float * pVertex, float * FaceScale,  float *pMaskUV, int nVertex)
{
	if (m_pMergeVertex == NULL)
	{
		m_pMergeVertex = new float[nVertex*m_nVertsSize];
	}
	for (int i = 0; i < nVertex; i++)
	{
		m_pMergeVertex[i * m_nVertsSize] = pVertex[i * 2];
		m_pMergeVertex[i * m_nVertsSize + 1] = pVertex[i * 2 + 1];
		m_pMergeVertex[i * m_nVertsSize + 2] = FaceScale[i * 2];
		m_pMergeVertex[i * m_nVertsSize + 3] = FaceScale[i * 2 + 1];
		m_pMergeVertex[i * m_nVertsSize + 4] = pMaskUV[i * 2];
		m_pMergeVertex[i * m_nVertsSize + 5] = pMaskUV[i * 2 + 1];
	}

// 	freopen("point.txt", "w", stdout);
//  	std::cout << nVertex << std::endl;
//  	for (int i = 0; i < nVertex; i++) {
//  		std::cout << pVertex[i * 2]<< "," << pVertex[i * 2+1] << ",";
//  	}
//  	fclose(stdout);

}

float* CFaceSPMLS::GetOffsetTexture()
{
	if (m_needUpdate)
	{
		m_needUpdate = false;
		return m_pOffestTexture;
	}
	else
	{
		return NULL;
	}
}

void CFaceSPMLS::UpdateWithOffestTexture(float * pOffestTexture)
{
	if (pOffestTexture!=NULL)
	{
		memcpy(m_pOffestTexture, pOffestTexture, m_SmallTextureHeight*m_SmallTextureWidth * sizeof(float) * 2);
		m_nBigSize = 0;
		SAFE_RELEASE_BUFFER(m_IndexBuffer2);
		SAFE_RELEASE_BUFFER(m_VerticeBuffer2);
	}

}

void CFaceSPMLS::SetOperate(CFaceSPMLS * part)
{
	if (m_hasCopy)
	{
		return;
	}
	m_hasCopy = true;
	SAFE_DELETE(m_FaceShape);
	SAFE_DELETE(m_EllipseShape);
	m_FaceShape = part->m_FaceShape;
	m_EllipseShape = part->m_EllipseShape;

}

void CFaceSPMLS::ReadConfig(XMLNode & childNode, HZIP hZip, char * pFilePath, const std::string& path)
{
	if (!childNode.isEmpty())
	{
		int j = -1;
		XMLNode nodeDrawable = childNode.getChildNode("drawable", ++j);
		SAFE_DELETE_ARRAY(m_pOffestTexture);
		m_pOffestTexture = new float[m_SmallTextureWidth*m_SmallTextureHeight * 2];
		memset(m_pOffestTexture, 0, sizeof(float)*m_SmallTextureWidth*m_SmallTextureHeight * 2);

		while (!nodeDrawable.isEmpty())
		{
			const char *szDrawableName = nodeDrawable.getAttribute("Offset");
			if (szDrawableName != NULL)
			{
				BYTE *pOffestTexture = NULL;
				if (hZip == 0)
				{
					std::string szFullFile = path + "/" + szDrawableName;
					pOffestTexture = DXUtils::CreateImgFromFile(szFullFile.c_str(), m_SmallTextureWidth, m_SmallTextureHeight);
				}
				else
				{
					pOffestTexture = DXUtils::CreateImgFromZIP(hZip, szDrawableName, m_SmallTextureWidth, m_SmallTextureHeight);
				}
				if (pOffestTexture != NULL)
				{
					int nSize = m_SmallTextureHeight * m_SmallTextureWidth;
					for (int i = 0; i < nSize; i++)
					{
						int pos = i * 4;
						float xc = pOffestTexture[pos + 0] * 255.f + pOffestTexture[pos + 1];
						float yc = pOffestTexture[pos + 2] * 255.f + pOffestTexture[pos + 3];
						xc /= 255.f;
						yc /= 255.f;
						xc -= 127;
						yc -= 127;
						m_pOffestTexture[i * 2] += xc;
						m_pOffestTexture[i * 2 + 1] += yc;
					}
					SAFE_DELETE_ARRAY(pOffestTexture);
				}

			}

			const char *szStyle = nodeDrawable.getAttribute("Style");
			if (szStyle != NULL)
			{
				m_Style = szStyle;
			}

			nodeDrawable = childNode.getChildNode("drawable", ++j);
		}
		if (hZip == 0)
		{
			if (!strcmp(m_Style.c_str(), "Mesh"))
			{
				if (m_FaceShape == NULL)
				{
					m_FaceShape = new FaceDeformation();
				}
				m_FaceShape->ReadConfig(childNode);
				m_bOffestTexture = m_FaceShape->GetOffsetTexture();
			}
			else
			{
				if (m_EllipseShape == NULL)
				{
					m_EllipseShape = new EllipseDeformation();
				}
				m_EllipseShape->ReadConfig(childNode);
				m_bOffestTexture = m_EllipseShape->GetOffsetTexture();
			}
		}

		//time
		XMLNode nodeTime = childNode.getChildNode("time", 0);
		if (!nodeTime.isEmpty())
		{
			const char* szDelay = nodeTime.getAttribute("delay");
			const char* szPlay = nodeTime.getAttribute("play");
			const char* szAll = nodeTime.getAttribute("all");
			if (szDelay != NULL)
			{
				m_play.delaytime = atoi(szDelay);
			}
			if (szPlay != NULL)
			{
				m_play.playtime = atoi(szPlay);
			}

			if (szAll != NULL)
			{
				m_play.alltime = atoi(szAll);
			}

		}
	}

}

bool CFaceSPMLS::WriteConfig(std::string &tempPath, XMLNode &root, HZIP dst, HZIP src)
{

	XMLNode nodeEffect = root.addChild("typeeffect");
	std::string typeEffect = "FaceMLS";
	nodeEffect.addAttribute("type", typeEffect.c_str());
	nodeEffect.addAttribute("showname", m_showname.c_str());
	XMLNode nodeDrwable = nodeEffect.addChild("drawable");
	std::string offsetName;

	if (!strcmp(m_Style.c_str(), "Mesh"))
	{
		m_bOffestTexture = m_FaceShape->GetOffsetTexture();
		int nameID = (int)&m_FaceShape;
		offsetName = "mesh_mls_" + to_string(nameID) + ".png";
		nodeDrwable.addAttribute("Offset", offsetName.c_str());
	}
	else
	{
		m_bOffestTexture = m_EllipseShape->GetOffsetTexture();
		int nameID = (int)&m_EllipseShape;
		offsetName = "ellipse_mls_" + to_string(nameID) + ".png";
		nodeDrwable.addAttribute("Offset", offsetName.c_str());
	}


	//XMLNode nodeDrwable = nodeEffect.addChild("drawable");
	//nodeDrwable.addAttribute("Offset", "mls_13.png");
	nodeDrwable.addAttribute("Style", m_Style.c_str());

	std::string imgPath = tempPath + "/" + offsetName;
	if (m_bOffestTexture != NULL)
	{
		stbi_write_png(imgPath.c_str(), m_SmallTextureWidth, m_SmallTextureHeight, 4, m_bOffestTexture, m_SmallTextureWidth * 4);
	}
	if (!strcmp(m_Style.c_str(), "Mesh"))
	{
		m_FaceShape->WriteConfig(nodeEffect);
	}
	else
	{
		m_EllipseShape->WriteConfig(nodeEffect);
	}
	

	return true;
}

void CFaceSPMLS::UpdateOffset()
{
	if (!strcmp(m_Style.c_str(), "Mesh"))
	{
		m_FaceShape->updateOffest();
		m_bOffestTexture = m_FaceShape->GetOffsetTexture();
	}
	else
	{
		m_EllipseShape->updateOffset();
		m_bOffestTexture = m_EllipseShape->GetOffsetTexture();
	}

	if (m_bOffestTexture == NULL)
	{
		return;
	}
	int nSize = m_SmallTextureHeight * m_SmallTextureWidth;
	for (int i = 0; i < nSize; i++)
	{
		int pos = i * 4;

		float xc = m_bOffestTexture[pos + 0] * 255.f + m_bOffestTexture[pos + 1];
		float yc = m_bOffestTexture[pos + 2] * 255.f + m_bOffestTexture[pos + 3];
		xc /= 255.f;
		yc /= 255.f;
		xc -= 127;
		yc -= 127;
		m_pOffestTexture[i * 2] = xc;
		m_pOffestTexture[i * 2 + 1] = yc;
	}

	m_nBigSize = 0;
	SAFE_RELEASE_BUFFER(m_IndexBuffer2);
	SAFE_RELEASE_BUFFER(m_VerticeBuffer2);
	m_needUpdate = true;
}

