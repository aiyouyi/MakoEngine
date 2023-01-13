#include "CFaceBeauty.h"
#include "Toolbox/DXUtils/DXUtils.h"
#include "BaseDefine/Define.h"
#include <algorithm>
#include <iostream>
#include "EffectKernel/ShaderProgramManager.h"
#include "Toolbox/DXUtils/DX11Resource.h"
#include "Toolbox/Render/DynamicRHI.h"


CFaceBeauty::CFaceBeauty()
{
	m_SmallTextureWidth = 80;
	m_SmallTextureHeight = 80;

	m_IndexBuffer = NULL;
	for (int i = 0; i < MAX_SUPPORT_PEOPLE; i++)
	{
		m_VerticeBuffer[i] = NULL;
	}
	for (int i = 0; i < MaxTypeCount; i++)
	{
		m_pOffestTexture[i] = NULL;
		m_alpha2[i] = 0;
	}
	m_Texture = NULL;
	m_nWidth = 0;
	m_nHeight = 0;
	m_VTPointCount = 0;
	m_InterFM = NULL;
	m_pMergeVertex = NULL;
	m_pConstantBuffer = NULL;
	m_EffectPart = FACE_LIFT_EFFECT;
}


CFaceBeauty::~CFaceBeauty()
{
	Release();
}

void CFaceBeauty::Release()
{
	SAFE_RELEASE_BUFFER(m_IndexBuffer);

	for (int i = 0; i < MAX_SUPPORT_PEOPLE; i++)
	{
		SAFE_RELEASE_BUFFER(m_VerticeBuffer[i]);
	}
	for (int i = 0; i < MaxTypeCount; i++)
	{
		SAFE_DELETE_ARRAY(m_pOffestTexture[i]);
	}
	SAFE_RELEASE_BUFFER(m_pConstantBuffer);
	SAFE_DELETE(m_InterFM);
	SAFE_DELETE_ARRAY(m_pMergeVertex);
	SAFE_DELETE(m_Texture);

}

void CFaceBeauty::SetAlpha(float alpha, CCEffectType type)
{
	int nType = (int)(type - FACE_LIFT_EFFECT);
	if (m_pOffestTexture[nType] != NULL && fabs(m_alpha2[nType] - alpha)>0.0001)
	{
		m_alpha2[nType] = alpha;
		GenerateTexture();
	}
	
}

void CFaceBeauty::Resize(int nWidth, int nHeight)
{
	m_nWidth = nWidth;
	m_nHeight = nHeight;

	int nBigSize = (std::min)(m_nWidth, m_nHeight);

}


void CFaceBeauty::RunFace106To118(Vector2 * pFacePoint, Vector2 *pFacePoint118)
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

void * CFaceBeauty::Clone()
{
	CFaceBeauty* result = new CFaceBeauty();
	*result = *this;
	return result;
}

void CFaceBeauty::initOffestTexture(CCEffectType type, BYTE *pOffestTexture)
{

	if (pOffestTexture != NULL)
	{
		int nType = (int)(type - FACE_LIFT_EFFECT);
		SAFE_DELETE_ARRAY(m_pOffestTexture[nType]);
		m_pOffestTexture[nType] = new float[m_SmallTextureWidth*m_SmallTextureHeight * 2];
		int nSize = m_SmallTextureHeight * m_SmallTextureWidth;
		float *pOffset = m_pOffestTexture[nType];
		for (int i = 0; i < nSize; i++)
		{
			int pos = i * 4;
			float xc = pOffestTexture[pos + 0] * 255.f + pOffestTexture[pos + 1];
			float yc = pOffestTexture[pos + 2] * 255.f + pOffestTexture[pos + 3];
			xc /= 255.f;
			yc /= 255.f;
			xc -= 127;
			yc -= 127;
			pOffset[i * 2] = xc;
			pOffset[i * 2 + 1] = yc;
		}
		SAFE_DELETE_ARRAY(pOffestTexture);
	}
}

bool CFaceBeauty::ReadConfig(XMLNode & childNode, HZIP hZip, char * pFilePath)
{
	if (!childNode.isEmpty())
	{
		XMLNode nodeDrawable = childNode.getChildNode("drawable", 0);


		if (!nodeDrawable.isEmpty())
		{
			const char *szDrawableName = nodeDrawable.getAttribute("facelift");
			if (szDrawableName != NULL)
			{
				BYTE *pOffestTexture = DXUtils::CreateImgFromZIP(hZip, szDrawableName, m_SmallTextureWidth, m_SmallTextureHeight);
				initOffestTexture(FACE_LIFT_EFFECT, pOffestTexture);
			}
			szDrawableName = nodeDrawable.getAttribute("facenarrow");
			if (szDrawableName != NULL)
			{
				BYTE *pOffestTexture = DXUtils::CreateImgFromZIP(hZip, szDrawableName, m_SmallTextureWidth, m_SmallTextureHeight);
				initOffestTexture(FACE_NARROW_EFFECT, pOffestTexture);
			}

			szDrawableName = nodeDrawable.getAttribute("facesmall");
			if (szDrawableName != NULL)
			{
				BYTE *pOffestTexture = DXUtils::CreateImgFromZIP(hZip, szDrawableName, m_SmallTextureWidth, m_SmallTextureHeight);
				initOffestTexture(FACE_SMALL_EFFECT, pOffestTexture);
			}

			szDrawableName = nodeDrawable.getAttribute("eyebig");
			if (szDrawableName != NULL)
			{
				BYTE *pOffestTexture = DXUtils::CreateImgFromZIP(hZip, szDrawableName, m_SmallTextureWidth, m_SmallTextureHeight);
				initOffestTexture(FACE_EYE_BIG_EFFECT, pOffestTexture);
			}

			szDrawableName = nodeDrawable.getAttribute("eyeoffset");
			if (szDrawableName != NULL)
			{
				BYTE *pOffestTexture = DXUtils::CreateImgFromZIP(hZip, szDrawableName, m_SmallTextureWidth, m_SmallTextureHeight);
				initOffestTexture(FACE_EYE_OFFEST_EFFECT, pOffestTexture);
			}

			szDrawableName = nodeDrawable.getAttribute("eyerotate");
			if (szDrawableName != NULL)
			{
				BYTE *pOffestTexture = DXUtils::CreateImgFromZIP(hZip, szDrawableName, m_SmallTextureWidth, m_SmallTextureHeight);
				initOffestTexture(FACE_EYE_ROTATE_EFFECT, pOffestTexture);
			}
			szDrawableName = nodeDrawable.getAttribute("forehead");
			if (szDrawableName != NULL)
			{
				BYTE *pOffestTexture = DXUtils::CreateImgFromZIP(hZip, szDrawableName, m_SmallTextureWidth, m_SmallTextureHeight);
				initOffestTexture(FACE_FOREHEAD_EFFECT, pOffestTexture);
			}
			szDrawableName = nodeDrawable.getAttribute("chin");
			if (szDrawableName != NULL)
			{
				BYTE *pOffestTexture = DXUtils::CreateImgFromZIP(hZip, szDrawableName, m_SmallTextureWidth, m_SmallTextureHeight);
				initOffestTexture(FACE_CHIN_EFFECT, pOffestTexture);
			}
			szDrawableName = nodeDrawable.getAttribute("nose");
			if (szDrawableName != NULL)
			{
				BYTE *pOffestTexture = DXUtils::CreateImgFromZIP(hZip, szDrawableName, m_SmallTextureWidth, m_SmallTextureHeight);
				initOffestTexture(FACE_NOSE_EFFECT, pOffestTexture);
			}
			szDrawableName = nodeDrawable.getAttribute("mouth");
			if (szDrawableName != NULL)
			{
				BYTE *pOffestTexture = DXUtils::CreateImgFromZIP(hZip, szDrawableName, m_SmallTextureWidth, m_SmallTextureHeight);
				initOffestTexture(FACE_MOUTH_EFFECT, pOffestTexture);
			}
			szDrawableName = nodeDrawable.getAttribute("nosepos");
			if (szDrawableName != NULL)
			{
				BYTE *pOffestTexture = DXUtils::CreateImgFromZIP(hZip, szDrawableName, m_SmallTextureWidth, m_SmallTextureHeight);
				initOffestTexture(FACE_NOSEPOS_EFFECT, pOffestTexture);
			}
			szDrawableName = nodeDrawable.getAttribute("mouthpos");
			if (szDrawableName != NULL)
			{
				BYTE *pOffestTexture = DXUtils::CreateImgFromZIP(hZip, szDrawableName, m_SmallTextureWidth, m_SmallTextureHeight);
				initOffestTexture(FACE_MOUTHPOS_EFFECT, pOffestTexture);
			}
			szDrawableName = nodeDrawable.getAttribute("lowerjaw");
			if (szDrawableName != NULL)
			{
				BYTE *pOffestTexture = DXUtils::CreateImgFromZIP(hZip, szDrawableName, m_SmallTextureWidth, m_SmallTextureHeight);
				initOffestTexture(FACE_LOWER_JAW, pOffestTexture);
			}
			szDrawableName = nodeDrawable.getAttribute("cheekbones");
			if (szDrawableName != NULL)
			{
				BYTE *pOffestTexture = DXUtils::CreateImgFromZIP(hZip, szDrawableName, m_SmallTextureWidth, m_SmallTextureHeight);
				initOffestTexture(FACE_CHEEK_BONES, pOffestTexture);
			}
			szDrawableName = nodeDrawable.getAttribute("yamane");//ɽ��
			if (szDrawableName != NULL)
			{
				BYTE *pOffestTexture = DXUtils::CreateImgFromZIP(hZip, szDrawableName, m_SmallTextureWidth, m_SmallTextureHeight);
				initOffestTexture(FACE_YAMANE_EFFECT, pOffestTexture);
			}
			szDrawableName = nodeDrawable.getAttribute("foreheadW");
			if (szDrawableName != NULL)
			{
				BYTE *pOffestTexture = DXUtils::CreateImgFromZIP(hZip, szDrawableName, m_SmallTextureWidth, m_SmallTextureHeight);
				initOffestTexture(FACE_FOREHEADW_EFFECT, pOffestTexture);
			}
		}

		GenerateTexture();
		return true;
	}
	return false;
}

bool CFaceBeauty::Prepare()
{
	CCVetexAttribute pAttribute[] =
	{
		{VERTEX_ATTRIB_POSITION, FLOAT_C4},
		{VERTEX_ATTRIB_TEX_COORD, FLOAT_C2}
	};
	string path = m_resourcePath + "/Shader/faceMLSByImg2.fx";
	m_pShader = ShaderProgramManager::GetInstance()->GetOrCreateShaderByPathAndAttribs(path, pAttribute, 2);

	m_InterFM = new InterFacePointDeform();
	m_nVertsSize = 6;
	return true;
}

void CFaceBeauty::GenerateTexture()
{
	SAFE_DELETE(m_Texture);
	int nSize = m_SmallTextureHeight * m_SmallTextureWidth;
	if (m_pOffestTexture[MaxTypeCount - 1]==NULL)
	{
		m_pOffestTexture[MaxTypeCount - 1] = new float[nSize * 2];
	}
	memset(m_pOffestTexture[MaxTypeCount - 1], 0, 2 * nSize*sizeof(float));
	BYTE *pData = new BYTE[nSize*4];
	memset(pData, 0, 4 * nSize);
	float *pSumOffest = m_pOffestTexture[MaxTypeCount - 1];
	for (int j = 0; j < MaxTypeCount; j++)
	{
		if (fabs(m_alpha2[j]) > 0.0001&&m_pOffestTexture[j]!=NULL)
		{
			float *pOffset = m_pOffestTexture[j];
			float alpha = m_alpha2[j];
			for (int i = 0; i < nSize; i++)
			{
				pSumOffest[i*2] += pOffset[i * 2 + 0]*alpha;
				pSumOffest[i * 2+1] += pOffset[i * 2 + 1]*alpha;
			}
		}
	}

// 	int n;
// 	BYTE *pMask = ccLoadImage("./maskProtect.png", &m_SmallTextureWidth, &m_SmallTextureHeight, &n, 4);
// 	for (int i = 0; i < nSize; i++)
// 	{
// 		float alpha = 1.0 - pMask[i*4] * 1.f / 255;
// 		pSumOffest[i * 2] *= alpha;
// 		pSumOffest[i * 2+1] *= alpha;
// 	}
// 	SAFE_DELETE_ARRAY(pMask);


	for (int i = 0; i < nSize; i++)
	{
		float x = pSumOffest[i * 2] + 127;
		float y = pSumOffest[i * 2 + 1] +127;

		int xi = x;
		int yi = y;
		int g = (x - xi) * 255;
		int a = (y - yi) * 255;
		pData[i * 4] = xi;
		pData[i * 4 + 1] = g;
		pData[i * 4 + 2] = yi;
		pData[i * 4+3] = a;

	}
//	ccSavePng("out.png", pData, m_SmallTextureWidth, m_SmallTextureHeight, 4);
	m_Texture = new DX11Texture();
	m_Texture->initTexture(DXGI_FORMAT_R8G8B8A8_UNORM, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET, m_SmallTextureWidth, m_SmallTextureHeight, pData, m_SmallTextureWidth*4, false, false);


	SAFE_DELETE_ARRAY(pData);
}

void GetEyeScale(Vector2 *pPoint118, float*pScaleVal)
{
	float LEyeHeight = pPoint118[53].distance(pPoint118[57]);// CalDistance(pPoint118[53].x, pPoint118[53].y, pPoint118[57].x, pPoint118[57].y);
	float LEyeWidth = pPoint118[53].distance(pPoint118[57]);//CalDistance(pPoint118[51].x, pPoint118[51].y, pPoint118[55].x, pPoint118[55].y);
	float REyeHeight = pPoint118[53].distance(pPoint118[57]);// CalDistance(pPoint118[63].x, pPoint118[63].y, pPoint118[67].x, pPoint118[67].y);
	float REyeWidth = pPoint118[53].distance(pPoint118[57]);//CalDistance(pPoint118[61].x, pPoint118[61].y, pPoint118[65].x, pPoint118[65].y);

	float UserW2H = (std::max)(LEyeHeight, REyeHeight) / (std::max)(LEyeWidth, REyeWidth);
	float StandardW2H = 0.402193159;
	float fW2HRatio = UserW2H / StandardW2H;
	float  S = fW2HRatio > 1.0f ? 1.0f : fW2HRatio;

	float fL2RHeightRatio = LEyeHeight / REyeHeight;
	if (fL2RHeightRatio > 1.0f)
	{
		pScaleVal[0] = 1.0f;
		pScaleVal[1] = 1.0f / fL2RHeightRatio;
	}
	else
	{
		pScaleVal[1] = 1.0f;
		pScaleVal[0] = fL2RHeightRatio;
	}
	pScaleVal[0] *= S;
	pScaleVal[1] *= S;

}



void CFaceBeauty::Render(BaseRenderParam & RenderParam)
{
	if (RenderParam.GetFaceCount() == 0)
	{
		return;
	}
//	DXUtils::SetRasterizerState(false);

	bool hasEffect = false;
	for (int i = 0; i < MaxTypeCount; i++) {
		if (fabs(m_alpha2[i]) > 0.001)
		{
			hasEffect = true;
		}
	}
	if (!hasEffect)
	{
		return;
	}

	Resize(RenderParam.GetWidth(), RenderParam.GetHeight());

	auto pDoubleBuffer = RenderParam.GetDoubleBuffer();

	

	int nFaceCount = RenderParam.GetFaceCount();
	for (int faceIndex = 0; faceIndex < nFaceCount; faceIndex++)
	{
	//	pDoubleBuffer->SyncAToB();
		m_pShader->useShader();
		pDoubleBuffer->BindFBOA();

		auto pMaterialView = m_Texture->getTexShaderView();
		DeviceContextPtr->PSSetSamplers(0, 1, &m_pSamplerLinear);
		DeviceContextPtr->PSSetShaderResources(1, 1, &pMaterialView);

		Vector2 pSrcpoint118[118];
		RunFace106To118(RenderParam.GetFacePoint(faceIndex, FACE_POINT_106),pSrcpoint118);
		Vector2 pSrcpoint442[442];
		m_InterFM->RunFace118To442(pSrcpoint118, 0, pSrcpoint442);
		float pTransformMatrix[8];
		GetOffsetTransform(pSrcpoint118,m_nWidth, m_nHeight, pTransformMatrix);
		float sideParam = GetSideFaceParam((float*)pSrcpoint118, 0.5);
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
		pDoubleBuffer->SyncAToBRegion((float*)pSrcpoint442, 442);
		//auto pSrcShaderView = pDoubleBuffer->GetFBOTextureB()->getTexShaderView();
		//DeviceContextPtr->PSSetShaderResources(0, 1, &pSrcShaderView);
		GetDynamicRHI()->SetPSShaderResource(0, RHIResourceCast(pDoubleBuffer.get())->GetFBOTextureB());

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


float CFaceBeauty::CalArea(Vector2 point1, Vector2 point2, Vector2 point3)
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

void CFaceBeauty::GetOffsetTransform(Vector2 *pPoint118, int nImgWidth, int nImgHeight, float *pOutMat)
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

void CFaceBeauty::GetFaceScaleEachPoint(Vector2 * pPoint118, Vector2 * pStandard442Src, unsigned short *pFaceMesh, Vector2 * pScaleValue442)
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
		//闭眼判断
		if (i < 10 || i == 169)
		{
			float DistNoseU = pPoint118[57].y - pPoint118[53].y;
			float DistNoseS = pPoint118[55].x - pPoint118[51].x;
			float minV = DistNoseU * 3 / DistNoseS;
			minV = std::min(minV, 1.0f);
			minV = std::max(0.f, minV);;
			ScaleiY = ScaleiY * minV;
		}
		else if (i < 20 || i == 170)
		{
			float DistNoseU = pPoint118[67].y - pPoint118[63].y;
			float DistNoseS = pPoint118[65].x - pPoint118[61].x;
			float minV = DistNoseU * 3 / DistNoseS;

			minV = std::min(minV, 1.0f);
			minV = std::max(0.f, minV);;
			ScaleiY = ScaleiY * minV;
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


float CFaceBeauty::GetSideFaceParam(float * pPoint118, float SideParam)
{
	Vector2 *Point118 = (Vector2*)pPoint118;

	float  leftDist = Point118[0].distance(Point118[71]);//CalDistance(pPoint118[0 * 2], pPoint118[0 * 2 + 1], pPoint118[71 * 2], pPoint118[71 * 2 + 1]);
	float rightDist = Point118[32].distance(Point118[71]);//CalDistance(pPoint118[32 * 2], pPoint118[32 * 2 + 1], pPoint118[71 * 2], pPoint118[71 * 2 + 1]);

	float minDist = (std::min)(leftDist, rightDist);
	float maxDist = (std::max)(leftDist, rightDist);

	float side = minDist / maxDist;

	return SideParam * (1 - side) + side;

}

void CFaceBeauty::MergeVertex(float * pVertex, float * FaceScale,  float *pMaskUV, int nVertex)
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