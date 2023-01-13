#include "CFaceSPMLSGL.h"
#include "BaseDefine/Define.h"
#include <algorithm>
#include <iostream>
#include"BaseDefine/commonFunc.h"
#include "Toolbox/GL/GLResource.h"
#include "Toolbox/Render/DynamicRHI.h"
#include "Toolbox/Render/MaterialTexRHI.h"

CFaceSPMLSGL::CFaceSPMLSGL()
{
	m_SmallTextureWidth = 80;
	m_SmallTextureHeight = 80;
    //m_pOffestTexture = NULL;
	m_nWidth = 0;
	m_nHeight = 0;
	m_InterFM = NULL;
}


CFaceSPMLSGL::~CFaceSPMLSGL()
{
	Release();
}

void CFaceSPMLSGL::Release()
{
	SAFE_DELETE(m_InterFM);
	SAFE_DELETE_ARRAY(m_OffestData);
	m_pShader.reset();
	m_pOffestTexture.reset();
}

void CFaceSPMLSGL::Resize(int nWidth, int nHeight)
{
	m_nWidth = nWidth;
	m_nHeight = nHeight;
}

void CFaceSPMLSGL::RunFace106To118(Vector2 * pFacePoint, Vector2 *pFacePoint118)
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

void * CFaceSPMLSGL::Clone()
{
	CFaceSPMLSGL* result = new CFaceSPMLSGL();
	*result = *this;
	return result;
}

bool CFaceSPMLSGL::ReadConfig(XMLNode & childNode, HZIP hZip, char * pFilePath)
{
	if (!childNode.isEmpty())
	{
		int j = -1;
		XMLNode nodeDrawable = childNode.getChildNode("drawable", ++j);
        float *m_pOffest = new float[m_SmallTextureWidth*m_SmallTextureHeight * 2];
        memset(m_pOffest, 0, sizeof(float)*m_SmallTextureWidth*m_SmallTextureHeight * 2);

		while (!nodeDrawable.isEmpty())
		{
			const char *szDrawableName = nodeDrawable.getAttribute("Offset");

			if (szDrawableName != NULL)
			{
                BYTE *pOffestTexture = CreateImgFromZIP(hZip, szDrawableName, m_SmallTextureWidth, m_SmallTextureHeight);
				if (pOffestTexture!=NULL)
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
                        m_pOffest[i * 2] += xc;
                        m_pOffest[i * 2 + 1] += yc;
					}
					SAFE_DELETE_ARRAY(pOffestTexture);
				}

			}


			nodeDrawable = childNode.getChildNode("drawable", ++j);
        }
        int nSize = m_SmallTextureHeight * m_SmallTextureWidth;

		SAFE_DELETE_ARRAY(m_OffestData);
		m_OffestData = new byte_t[nSize * 4];
        //m_pOffestTexture.m_pData = new BYTE[nSize*4];
        //m_pOffestTexture.m_Realese = true;
        //m_pOffestTexture.m_nWidth = m_SmallTextureWidth;
        //m_pOffestTexture.m_nHeight = m_SmallTextureHeight;
        BYTE *pData = m_OffestData;

        for (int i = 0; i < nSize; i++)
        {
            float x = m_pOffest[i * 2] + 127;
            float y = m_pOffest[i * 2 + 1] +127;

            int xi = x;
            int yi = y;
            int g = (x - xi) * 255;
            int a = (y - yi) * 255;
            pData[i * 4] = xi;
            pData[i * 4 + 1] = g;
            pData[i * 4 + 2] = yi;
            pData[i * 4+3] = a;

        }
		m_pOffestTexture = GetDynamicRHI()->CreateAsynTexture(CC3DTextureRHI::SFT_A8R8G8B8, 0, m_SmallTextureWidth, m_SmallTextureHeight, m_OffestData, m_SmallTextureWidth*4);
		delete []m_pOffest;
		return true;
	}
	return false;
}

bool CFaceSPMLSGL::Prepare()
{
	m_pShader = std::make_shared<CCProgram>();
    std::string  vspath = m_resourcePath + "/Shader/faceMLSByImg.vs";
    std::string  fspath = m_resourcePath + "/Shader/faceMLSByImg2.fs";
    m_pShader->CreatePorgramForFile(vspath.c_str(),fspath.c_str());
	m_InterFM = new InterFacePointDeform();
	return true;
}

void CFaceSPMLSGL::Render(BaseRenderParam & RenderParam)
{
    if(m_alpha<0.001f)
    {
        return;
    }

	if (RenderParam.GetFaceCount() == 0)
	{
		return;
	}
	Resize(RenderParam.GetWidth(), RenderParam.GetHeight());
	auto pDoubleBuffer = RenderParam.GetDoubleBuffer();

	int nFaceCount = RenderParam.GetFaceCount();

	for (int faceIndex = 0; faceIndex < nFaceCount; faceIndex++)
	{
        //pDoubleBuffer->SyncAToB();
        Vector2 pSrcpoint118[118];
        RunFace106To118(RenderParam.GetFacePoint(faceIndex, FACE_POINT_106),pSrcpoint118);
        Vector2 pSrcpoint442[442];
        m_InterFM->RunFace118To442(pSrcpoint118, 0, pSrcpoint442);
        float pTransformMatrix[4];
        GetOffsetTransform(pSrcpoint118,m_nWidth, m_nHeight, pTransformMatrix);
        float sideParam = GetSideFaceParam((float*)pSrcpoint118, 0.5)*m_alpha;
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
		m_pShader->Use();
		glViewport(0, 0, RenderParam.GetWidth(), RenderParam.GetHeight());
        m_pShader->SetVertexAttribPointer("a_position", 2, GL_FLOAT, false, 0, (float*)pSrcpoint442);
        m_pShader->SetVertexAttribPointer("a_FaceScaleRadius", 2, GL_FLOAT, false, 0, (float*)pScaleValue442);
        m_pShader->SetVertexAttribPointer("a_texcoordMask", 2, GL_FLOAT, false, 0, (float*)m_InterFM->m_StandDstPoint);
		m_pOffestTexture->Bind(1);
        m_pShader->SetUniform1i("inputImageTexture2", 1);

		GetDynamicRHI()->SetPSShaderResource(0, pDoubleBuffer->GetFBOTextureB());
		m_pShader->SetUniform1i("inputImageTexture", 0);

        m_pShader->SetUniformMatrix2fv("transformMatrix",pTransformMatrix, false, 1);
        glDrawElements(GL_TRIANGLES, 832*3, GL_UNSIGNED_SHORT, m_InterFM->m_StandIndex);
	}
} 

float CFaceSPMLSGL::CalArea(Vector2 point1, Vector2 point2, Vector2 point3)
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

void CFaceSPMLSGL::GetOffsetTransform(Vector2 *pPoint118, int nImgWidth, int nImgHeight, float *pOutMat)
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

void CFaceSPMLSGL::GetFaceScaleEachPoint(Vector2 * pPoint118, Vector2 * pStandard442Src, unsigned short *pFaceMesh, Vector2 * pScaleValue442)
{
	Vector2 pPoint442User[442];
	Vector2 pPoint442Standard[442];
	m_InterFM->RunFace118To442((Vector2*)pPoint118, 0, pPoint442User, false, true);

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

float CFaceSPMLSGL::GetSideFaceParam(float * pPoint118, float SideParam)
{
	Vector2 *Point118 = (Vector2*)pPoint118;

	float  leftDist = Point118[0].distance(Point118[71]);//CalDistance(pPoint118[0 * 2], pPoint118[0 * 2 + 1], pPoint118[71 * 2], pPoint118[71 * 2 + 1]);
	float rightDist = Point118[32].distance(Point118[71]);//CalDistance(pPoint118[32 * 2], pPoint118[32 * 2 + 1], pPoint118[71 * 2], pPoint118[71 * 2 + 1]);

	float minDist = (std::min)(leftDist, rightDist);
	float maxDist = (std::max)(leftDist, rightDist);

	float side = minDist / maxDist;

	return SideParam * (1 - side) + side;

}
