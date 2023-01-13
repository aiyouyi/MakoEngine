#include "CFaceBeautyGL.h"
#include "BaseDefine/Define.h"
#include <algorithm>
#include <iostream>
#include "BaseDefine/commonFunc.h"
#include "Toolbox/GL/CCProgram.h"
#include "Toolbox/GL/GLResource.h"
#include "Toolbox/Render/DynamicRHI.h"

CFaceBeautyGL::CFaceBeautyGL()
{
	m_SmallTextureWidth = 80;
	m_SmallTextureHeight = 80;

	for (int i = 0; i < MaxTypeCount; i++)
	{
		m_pOffestTexture[i] = NULL;
		m_alpha2[i] = 0;
	}
	m_Texture = 0;
	m_nWidth = 0;
	m_nHeight = 0;
	m_VTPointCount = 0;
	m_InterFM = NULL;

	m_EffectPart = FACE_LIFT_EFFECT;
}


CFaceBeautyGL::~CFaceBeautyGL()
{
	Release();
}

void CFaceBeautyGL::Release()
{
	for (int i = 0; i < MaxTypeCount; i++)
	{
		SAFE_DELETE_ARRAY(m_pOffestTexture[i]);
	}

	SAFE_DELETE(m_InterFM);
	GL_DELETE_TEXTURE(m_Texture);
	m_pShader.reset();
}

void CFaceBeautyGL::SetAlpha(float alpha, CCEffectType type)
{
	int nType = (int)(type-FACE_LIFT_EFFECT);
	if (m_pOffestTexture[nType] != NULL && fabs(m_alpha2[nType] - alpha)>0.0001)
	{
		m_alpha2[nType] = alpha;
		GenerateTexture();
	}
	
}

void CFaceBeautyGL::Resize(int nWidth, int nHeight)
{
	m_nWidth = nWidth;
	m_nHeight = nHeight;

	int nBigSize = (std::min)(m_nWidth, m_nHeight);
}


void CFaceBeautyGL::RunFace106To118(Vector2 * pFacePoint, Vector2 *pFacePoint118)
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

void * CFaceBeautyGL::Clone()
{
	CFaceBeautyGL* result = new CFaceBeautyGL();
	*result = *this;
	return result;
}

void CFaceBeautyGL::initOffestTexture(CCEffectType type, BYTE *pOffestTexture)
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

bool CFaceBeautyGL::ReadConfig(XMLNode & childNode, HZIP hZip, char * pFilePath)
{
	if (!childNode.isEmpty())
	{
		XMLNode nodeDrawable = childNode.getChildNode("drawable", 0);


		if (!nodeDrawable.isEmpty())
		{
			const char *szDrawableName = nodeDrawable.getAttribute("facelift");//瘦脸
			if (szDrawableName != NULL)
			{
				BYTE *pOffestTexture = CreateImgFromZIP(hZip, szDrawableName, m_SmallTextureWidth, m_SmallTextureHeight);
				initOffestTexture(FACE_LIFT_EFFECT, pOffestTexture);
				int nType = (int)FACE_LIFT_EFFECT;
           //     m_alpha2[nType] = 0.5;
			}
			szDrawableName = nodeDrawable.getAttribute("facenarrow");//窄脸
			if (szDrawableName != NULL)
			{
				BYTE *pOffestTexture = CreateImgFromZIP(hZip, szDrawableName, m_SmallTextureWidth, m_SmallTextureHeight);
				initOffestTexture(FACE_NARROW_EFFECT, pOffestTexture);
				int nType = (int)FACE_NARROW_EFFECT;
            //    m_alpha2[nType] = 0.3;
			}
			szDrawableName = nodeDrawable.getAttribute("facesmall");//小脸
			if (szDrawableName != NULL)
			{
				BYTE *pOffestTexture = CreateImgFromZIP(hZip, szDrawableName, m_SmallTextureWidth, m_SmallTextureHeight);
				initOffestTexture(FACE_SMALL_EFFECT, pOffestTexture);
				int nType = (int)FACE_SMALL_EFFECT;
             //   m_alpha2[nType] = 0.3;
			}


			szDrawableName = nodeDrawable.getAttribute("eyebig");//大眼
			if (szDrawableName != NULL)
			{
				BYTE *pOffestTexture = CreateImgFromZIP(hZip, szDrawableName, m_SmallTextureWidth, m_SmallTextureHeight);
				initOffestTexture(FACE_EYE_BIG_EFFECT, pOffestTexture);
				int nType = (int)FACE_EYE_BIG_EFFECT;
			//	m_alpha2[nType] = 0.5;
			}

			szDrawableName = nodeDrawable.getAttribute("eyeoffset");//眼距
			if (szDrawableName != NULL)
			{
				BYTE *pOffestTexture = CreateImgFromZIP(hZip, szDrawableName, m_SmallTextureWidth, m_SmallTextureHeight);
				initOffestTexture(FACE_EYE_OFFEST_EFFECT, pOffestTexture);
			}

			szDrawableName = nodeDrawable.getAttribute("eyerotate");//眼角
			if (szDrawableName != NULL)
			{
				BYTE *pOffestTexture = CreateImgFromZIP(hZip, szDrawableName, m_SmallTextureWidth, m_SmallTextureHeight);
				initOffestTexture(FACE_EYE_ROTATE_EFFECT, pOffestTexture);
			}
			szDrawableName = nodeDrawable.getAttribute("forehead");//额头/发际线
			if (szDrawableName != NULL)
			{
				BYTE *pOffestTexture = CreateImgFromZIP(hZip, szDrawableName, m_SmallTextureWidth, m_SmallTextureHeight);
				initOffestTexture(FACE_FOREHEAD_EFFECT, pOffestTexture);
			}
			szDrawableName = nodeDrawable.getAttribute("chin");//下巴
			if (szDrawableName != NULL)
			{
				BYTE *pOffestTexture = CreateImgFromZIP(hZip, szDrawableName, m_SmallTextureWidth, m_SmallTextureHeight);
				initOffestTexture(FACE_CHIN_EFFECT, pOffestTexture);
			}
			szDrawableName = nodeDrawable.getAttribute("nose");//瘦鼻
			if (szDrawableName != NULL)
			{
				BYTE *pOffestTexture = CreateImgFromZIP(hZip, szDrawableName, m_SmallTextureWidth, m_SmallTextureHeight);
				initOffestTexture(FACE_NOSE_EFFECT, pOffestTexture);
			}
			szDrawableName = nodeDrawable.getAttribute("mouth");//嘴巴
			if (szDrawableName != NULL)
			{
				BYTE *pOffestTexture = CreateImgFromZIP(hZip, szDrawableName, m_SmallTextureWidth, m_SmallTextureHeight);
				initOffestTexture(FACE_MOUTH_EFFECT, pOffestTexture);
			}

			szDrawableName = nodeDrawable.getAttribute("nosepos");//鼻长
			if (szDrawableName != NULL)
			{
				BYTE *pOffestTexture = CreateImgFromZIP(hZip, szDrawableName, m_SmallTextureWidth, m_SmallTextureHeight);
				initOffestTexture(FACE_NOSEPOS_EFFECT, pOffestTexture);
			}
			szDrawableName = nodeDrawable.getAttribute("mouthpos");//人中
			if (szDrawableName != NULL)
			{
				BYTE *pOffestTexture = CreateImgFromZIP(hZip, szDrawableName, m_SmallTextureWidth, m_SmallTextureHeight);
				initOffestTexture(FACE_MOUTHPOS_EFFECT, pOffestTexture);
			}
			szDrawableName = nodeDrawable.getAttribute("yamane");//山根
			if (szDrawableName != NULL)
			{
				BYTE *pOffestTexture = CreateImgFromZIP(hZip, szDrawableName, m_SmallTextureWidth, m_SmallTextureHeight);
				initOffestTexture(FACE_YAMANE_EFFECT, pOffestTexture);
			}
			szDrawableName = nodeDrawable.getAttribute("Checkbone");//颧骨
			if (szDrawableName != NULL)
			{
				BYTE *pOffestTexture = CreateImgFromZIP(hZip, szDrawableName, m_SmallTextureWidth, m_SmallTextureHeight);
				initOffestTexture(FACE_CHEEK_BONES, pOffestTexture);
			}
			szDrawableName = nodeDrawable.getAttribute("Jaw");//下颌骨
			if (szDrawableName != NULL)
			{
				BYTE *pOffestTexture = CreateImgFromZIP(hZip, szDrawableName, m_SmallTextureWidth, m_SmallTextureHeight);
				initOffestTexture(FACE_LOWER_JAW, pOffestTexture);
			}
		}

		//GenerateTexture();
		return true;
	}
	return false;
}

bool CFaceBeautyGL::Prepare()
{
	m_pShader = std::make_shared<class CCProgram>();
	std::string  vspath = m_resourcePath + "/Shader/faceMLSByImg.vs";
	std::string  fspath = m_resourcePath + "/Shader/faceMLSByImg2.fs";
	m_pShader->CreatePorgramForFile(vspath.c_str(), fspath.c_str());

	m_InterFM = new InterFacePointDeform();
	return true;
}

void CFaceBeautyGL::GenerateTexture()
{
	GL_DELETE_TEXTURE(m_Texture);
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
	OpenGLUtil::createToTexture(m_Texture, m_SmallTextureWidth, m_SmallTextureHeight, pData);
	SAFE_DELETE_ARRAY(pData);
}

extern void GetEyeScale(Vector2* pPoint118, float* pScaleVal);

void CFaceBeautyGL::Render(BaseRenderParam& RenderParam)
{
	if (RenderParam.GetFaceCount() == 0)
	{
		return;
	}
    bool hasEffect = false;
    for (int i=0;i<MaxTypeCount;i++) {
        if(fabs(m_alpha2[i])>0.001)
        {
            hasEffect = true;
        }
    }
    if(!hasEffect)
    {
        return;
    }
	if (isFirst)
	{
		GenerateTexture();
		isFirst = false;
	}
	if (m_Texture == 0)
	{
		return;
	}
	Resize(RenderParam.GetWidth(), RenderParam.GetHeight());

	auto pDoubleBuffer = RenderParam.GetDoubleBuffer();

	int nFaceCount = RenderParam.GetFaceCount();

	for (int faceIndex = 0; faceIndex < nFaceCount; faceIndex++)
	{
	//	pDoubleBuffer->SyncAToB();
		m_pShader->Use();
		glViewport(0, 0, RenderParam.GetWidth(), RenderParam.GetHeight());

		Vector2 pSrcpoint118[118];
		RunFace106To118(RenderParam.GetFacePoint(faceIndex, FACE_POINT_106), pSrcpoint118);
		Vector2 pSrcpoint442[442];
		m_InterFM->RunFace118To442(pSrcpoint118, 0, pSrcpoint442);
		float pTransformMatrix[4];
		GetOffsetTransform(pSrcpoint118, m_nWidth, m_nHeight, pTransformMatrix);
		float sideParam = GetSideFaceParam((float*)pSrcpoint118, 0.5);
		pTransformMatrix[0] *= sideParam;
		pTransformMatrix[1] *= sideParam;
		pTransformMatrix[2] *= sideParam;
		pTransformMatrix[3] *= sideParam;
		Vector2 pScaleValue442[442];
		GetFaceScaleEachPoint(pSrcpoint118, m_InterFM->m_StandStrPoint, m_InterFM->m_StandIndex, pScaleValue442);
		for (int i = 0; i < 442; i++) {
			pSrcpoint442[i].x = pSrcpoint442[i].x / m_nWidth;
			pSrcpoint442[i].y = pSrcpoint442[i].y / m_nHeight;
		}

		pDoubleBuffer->SyncAToBRegion((float*)pSrcpoint442, 442);
		pDoubleBuffer->BindFBOA();

		m_pShader->SetVertexAttribPointer("a_position", 2, GL_FLOAT, false, 0, (float*)pSrcpoint442);
		m_pShader->SetVertexAttribPointer("a_FaceScaleRadius", 2, GL_FLOAT, false, 0, (float*)pScaleValue442);
		m_pShader->SetVertexAttribPointer("a_texcoordMask", 2, GL_FLOAT, false, 0, (float*)m_InterFM->m_StandDstPoint);

		GetDynamicRHI()->SetPSShaderResource(0, pDoubleBuffer->GetFBOTextureB());
		m_pShader->SetUniform1i("inputImageTexture", 0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, m_Texture);
		m_pShader->SetUniform1i("inputImageTexture2", 1);
		m_pShader->SetUniformMatrix2fv("transformMatrix", pTransformMatrix, false, 1);
		glDrawElements(GL_TRIANGLES, 832 * 3, GL_UNSIGNED_SHORT, m_InterFM->m_StandIndex);
	}

//	SetRasterizerState(true);
} 


float CFaceBeautyGL::CalArea(Vector2 point1, Vector2 point2, Vector2 point3)
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

void CFaceBeautyGL::GetOffsetTransform(Vector2 *pPoint118, int nImgWidth, int nImgHeight, float *pOutMat)
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

void CFaceBeautyGL::GetFaceScaleEachPoint(Vector2 * pPoint118, Vector2 * pStandard442Src, unsigned short *pFaceMesh, Vector2 * pScaleValue442)
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
			minV = (std::min)(minV, 1.0f);
			minV = (std::max)(0.f, minV);;
			ScaleiY = ScaleiY * minV;
		}
		else if (i < 20 || i == 170)
		{
			float DistNoseU = pPoint118[67].y - pPoint118[63].y;
			float DistNoseS = pPoint118[65].x - pPoint118[61].x;
			float minV = DistNoseU * 3 / DistNoseS;

			minV = (std::min)(minV, 1.0f);
			minV = (std::max)(0.f, minV);;
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


float CFaceBeautyGL::GetSideFaceParam(float * pPoint118, float SideParam)
{
	Vector2 *Point118 = (Vector2*)pPoint118;

	float  leftDist = Point118[0].distance(Point118[71]);//CalDistance(pPoint118[0 * 2], pPoint118[0 * 2 + 1], pPoint118[71 * 2], pPoint118[71 * 2 + 1]);
	float rightDist = Point118[32].distance(Point118[71]);//CalDistance(pPoint118[32 * 2], pPoint118[32 * 2 + 1], pPoint118[71 * 2], pPoint118[71 * 2 + 1]);

	float minDist = (std::min)(leftDist, rightDist);
	float maxDist = (std::max)(leftDist, rightDist);

	float side = minDist / maxDist;

	return SideParam * (1 - side) + side;

}
