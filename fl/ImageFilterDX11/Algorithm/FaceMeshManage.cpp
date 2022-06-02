#include "FaceMeshManage.h"
#include "Algorithm/MLS.h"
//#include "Algorithm/MouthManage.h"
#include "Algorithm/MathUtils.h"
#include <iostream>

FaceMeshManage::FaceMeshManage()
{
	m_FixLength = 0;
	m_MeshData = NULL;
	m_IsMouthOpened = true;
	m_EyeAlpha = 0;
}


FaceMeshManage::~FaceMeshManage()
{
	SAFE_DELETE(m_MeshData);
}

FaceMeshData * FaceMeshManage::GetMesh(float * pFacePoint106, int nWidth, int nHeight, FaceMeshType eMeshType)
{
	if (m_MeshData == NULL)
	{
		m_MeshData = new FaceMeshData();
	}
	m_Vertices.clear();
	m_nWidth = nWidth;
	m_nHeight = nHeight;

	switch (eMeshType)
	{
	case MESH_EYEBROW:
		if (m_isCrop)
		{
			m_MeshData->m_pUV = (Vector2*)m_EyeBrowUVCrop.data();
		}
		else
		{
			m_MeshData->m_pUV = (Vector2*)m_EyeBrowUV.data();
		}
		m_MeshData->m_nVerts = m_EyeBrowUV.size() / 2;
		m_MeshData->m_pTriangle = m_EyeBrowIndex.data();
		m_MeshData->m_nTriangle = m_EyeBrowIndex.size() / 3;
		m_Vertices.resize(m_MeshData->m_nVerts);
		EyeBrowInter((Vector2*)pFacePoint106, m_Vertices.data());
		m_MeshData->m_pVertices = m_Vertices.data();
		break;
	case MESH_EYE: case MESH_EYE_LID:
		if (m_isCrop)
		{
			m_MeshData->m_pUV = (Vector2*)m_EyeUVCrop.data();
		}
		else
		{
			m_MeshData->m_pUV = (Vector2*)m_EyeUV.data();
		}
		m_MeshData->m_nVerts = m_EyeUV.size() / 2;
		m_MeshData->m_pTriangle = m_EyeIndex.data();
		m_MeshData->m_nTriangle = m_EyeIndex.size() / 3;
		m_Vertices.resize(m_MeshData->m_nVerts);
		EyeInter((Vector2*)pFacePoint106, m_Vertices.data());
		m_MeshData->m_pVertices = m_Vertices.data();
		break;
	case MESH_MOUTH:
		if (m_isCrop)
		{
			m_MeshData->m_pUV = (Vector2*)m_MouthUVCrop.data();
		}
		else
		{
			m_MeshData->m_pUV = (Vector2*)m_MouthUV.data();
		}
		m_MeshData->m_nVerts = m_MouthUV.size() / 2;
		m_MeshData->m_pTriangle = m_MouthIndex.data();
		m_MeshData->m_nTriangle = m_MouthIndex.size() / 3;
		m_Vertices.resize(m_MeshData->m_nVerts);
		MouthInter((Vector2*)pFacePoint106, m_Vertices.data());
		m_MeshData->m_pVertices = m_Vertices.data();
		break;
	default:
		break;
	}
	return m_MeshData;
}

int FaceMeshManage::GetMaxVerts()
{
	return m_EyeUV.size() / 2;
}


void FaceMeshManage::EyeBrowInter(Vector2 * pFacePoint, Vector2 * pEyeBrowPoint)
{
	//left
	{
		memcpy(pEyeBrowPoint, pFacePoint + 33, 9 * sizeof(Vector2));
		pEyeBrowPoint[13] = pFacePoint[33] + pFacePoint[37] - pFacePoint[38];

		Vector2 *pSrcEyeBrowPoint = (Vector2 *)m_EyeBrowUV.data();
		Vector2 ScaleSize = Vector2(m_nWidth, m_nWidth * 800 / 650);
		Vector2 Translate = pSrcEyeBrowPoint[2] * ScaleSize - pEyeBrowPoint[2];
		Vector2 SrcPoint[4] = {
			pSrcEyeBrowPoint[9] * ScaleSize - Translate,
			pSrcEyeBrowPoint[10] * ScaleSize - Translate,
			pSrcEyeBrowPoint[11] * ScaleSize - Translate,
			pSrcEyeBrowPoint[12] * ScaleSize - Translate,
		};
		Vector2 DstPoint[4];

		Vector2 ControlSrc[4] =
		{
			pSrcEyeBrowPoint[0] * ScaleSize - Translate,
			pSrcEyeBrowPoint[2] * ScaleSize - Translate,
			pSrcEyeBrowPoint[5] * ScaleSize - Translate,
			pSrcEyeBrowPoint[7] * ScaleSize - Translate,
		};
		Vector2 ControlDst[4] =
		{
			pEyeBrowPoint[0],
			pEyeBrowPoint[2],
			pEyeBrowPoint[5],
			pEyeBrowPoint[7],
		};
		TransformVertexMls(SrcPoint, DstPoint, ControlSrc, ControlDst, 4, 4);
		memcpy(pEyeBrowPoint + 9, DstPoint, 4 * sizeof(Vector2));
	}
	//right
	{
		memcpy(pEyeBrowPoint + 14, pFacePoint + 42, 9 * sizeof(Vector2));
		pEyeBrowPoint[27] = pFacePoint[46] + pFacePoint[42] - pFacePoint[50];

		Vector2 *pSrcEyeBrowPoint = (Vector2 *)m_EyeBrowUV.data();
		Vector2 ScaleSize = Vector2(m_nWidth, m_nWidth * 800 / 650);
		Vector2 Translate = pSrcEyeBrowPoint[16] * ScaleSize - pEyeBrowPoint[16];
		Vector2 SrcPoint[4] = {
			pSrcEyeBrowPoint[23] * ScaleSize - Translate,
			pSrcEyeBrowPoint[24] * ScaleSize - Translate,
			pSrcEyeBrowPoint[25] * ScaleSize - Translate,
			pSrcEyeBrowPoint[26] * ScaleSize - Translate,
		};
		Vector2 DstPoint[4];

		Vector2 ControlSrc[4] =
		{
			pSrcEyeBrowPoint[14] * ScaleSize - Translate,
			pSrcEyeBrowPoint[16] * ScaleSize - Translate,
			pSrcEyeBrowPoint[18] * ScaleSize - Translate,
			pSrcEyeBrowPoint[20] * ScaleSize - Translate,
		};
		Vector2 ControlDst[4] =
		{
			pEyeBrowPoint[14],
			pEyeBrowPoint[16],
			pEyeBrowPoint[18],
			pEyeBrowPoint[20],
		};
		TransformVertexMls(SrcPoint, DstPoint, ControlSrc, ControlDst, 4, 4);
		memcpy(pEyeBrowPoint + 23, DstPoint, 4 * sizeof(Vector2));
	}

	Vector2 ScaleSize(1.0 / m_nWidth, 1.0 / m_nHeight);
	for (int i = 0; i < 28; i++)
	{
		pEyeBrowPoint[i] = pEyeBrowPoint[i] * ScaleSize * 2.0 - Vector2(1.0, 1.0);
	}

	/*freopen("point_eyebrow.txt", "w", stdout);
	int nVerts = 28;
	std::cout << nVerts *2<< std::endl;
	Vector2 EyeP[28];
	for (int i = 0; i < nVerts; i++) {
		//std::cout << pEyePoint[i].x*0.5+0.5<< "f" << "," << pEyePoint[i].y*0.5+0.5 << "f" << ",";
		EyeP[i].x = pEyeBrowPoint[i].x * 0.5 + 0.5;
		EyeP[i].y = pEyeBrowPoint[i].y * 0.5 + 0.5;
	}

	for (int i = 0; i < nVerts; i++) {
		float x = (EyeP[i].x*m_nWidth - 232) / 465;
		float y = (EyeP[i].y*m_nHeight - 372) / 248;
		if (x < 0.0) { x = (float)0.0; }
		if (x > 1.0) { x = (float)1.0; }
		if (y < 0.0) { y = (float)0.0; }
		if (y > 1.0) { y = (float)1.0; }
		std::cout << x << "f" <<"," << y << "f" << ",";
	}

	std::cout << std::endl;
	std::cout << std::endl;
	fclose(stdout);*/

}

float MouthOpenParam(Vector2 *pFacePoints, float param = 0.25)
{
	//�������´��ľ������ȡ��ֵ
	float Ist = 0;
	float upans = pFacePoints[89].distance(pFacePoints[100]);
	float downans = pFacePoints[95].distance(pFacePoints[104]);
	float ans = (upans + downans) / 2.0f;
	//�촽�м����
	float midans = pFacePoints[100].distance(pFacePoints[104]);
	//�����촽����
	float lengthans = pFacePoints[98].distance(pFacePoints[102]) / 4.5f;
	if (lengthans < ans) {
		ans = lengthans;
	}
	//����
	float ratio = midans / ans;
	if (ratio < param) {
		Ist = 0.0;
	}
	else
	{
		Ist = ratio / param - 1.0;
	}
	return Ist;
}

void FaceMeshManage::MouthInter(Vector2 * pFacePoint, Vector2 * pMouthPoint)
{
	//m_IsMouthOpened = isMouthOpened(pFacePoint, 0.25);

	memcpy(pMouthPoint, pFacePoint + 86, 12 * sizeof(Vector2));
	memcpy(pMouthPoint + 12, pFacePoint + 99, 3 * sizeof(Vector2));
	memcpy(pMouthPoint + 15, pFacePoint + 103, 3 * sizeof(Vector2));
	memcpy(pMouthPoint + 38, pFacePoint + 106, 8 * sizeof(Vector2));
	pMouthPoint[46] = pMouthPoint[0] * 0.5 + pMouthPoint[12] * 0.5;
	pMouthPoint[47] = pMouthPoint[6] * 0.5 + pMouthPoint[14] * 0.5;
	pMouthPoint[48] = pMouthPoint[0] * 0.5 + pMouthPoint[17] * 0.5;
	pMouthPoint[49] = pMouthPoint[6] * 0.5 + pMouthPoint[15] * 0.5;

	float alphaMouth = MouthOpenParam(pFacePoint, 0.25);
	if (alphaMouth < 1.0)
	{
		pMouthPoint[12] = pMouthPoint[17] * (1.0 - alphaMouth) + pMouthPoint[12] * alphaMouth;
		pMouthPoint[13] = pMouthPoint[16] * (1.0 - alphaMouth) + pMouthPoint[13] * alphaMouth;
		pMouthPoint[14] = pMouthPoint[15] * (1.0 - alphaMouth) + pMouthPoint[14] * alphaMouth;

		pMouthPoint[46] = pMouthPoint[48] * (1.0 - alphaMouth) + pMouthPoint[46] * alphaMouth;
		pMouthPoint[38] = pMouthPoint[40] * (1.0 - alphaMouth) + pMouthPoint[38] * alphaMouth;
		pMouthPoint[39] = pMouthPoint[41] * (1.0 - alphaMouth) + pMouthPoint[39] * alphaMouth;
		pMouthPoint[47] = pMouthPoint[49] * (1.0 - alphaMouth) + pMouthPoint[47] * alphaMouth;
	}

	float avgLength = 0.0;
	for (int i = 1; i < 12; ++i) {
		avgLength += pMouthPoint[i].distance(pMouthPoint[i - 1]);
	}
	avgLength /= 11;
	avgLength *= 0.8;
	for (int i = 18; i < 23; i++)
	{
		pMouthPoint[i] = CalVerticalWithLength(pMouthPoint[i - 18], pMouthPoint[i - 17], -avgLength);
	}
	pMouthPoint[23] = CalVerticalWithLength(pMouthPoint[5], pMouthPoint[4], avgLength*1.5);
	pMouthPoint[24] = CalVerticalWithLength(pMouthPoint[6], pMouthPoint[25], avgLength);

	for (int i = 25; i < 30; i++)
	{
		pMouthPoint[i] = CalVerticalWithLength(pMouthPoint[i - 19], pMouthPoint[i - 18], -avgLength);
	}
	pMouthPoint[30] = CalVerticalWithLength(pMouthPoint[11], pMouthPoint[0], -avgLength);
	pMouthPoint[31] = CalVerticalWithLength(pMouthPoint[0], pMouthPoint[11], avgLength);

	Vector2 p1 = pMouthPoint[21] + (pMouthPoint[21] - pMouthPoint[3])*0.5;
	Vector2 p2 = pMouthPoint[28] + (pMouthPoint[28] - pMouthPoint[9])*2.0;
	Vector2 p3 = pMouthPoint[0] - (pMouthPoint[6] - pMouthPoint[0])*0.3;
	Vector2 p4 = pMouthPoint[6] - (pMouthPoint[0] - pMouthPoint[6])*0.3;
	pMouthPoint[32] = p3 - (p2 - p1)*0.5;
	pMouthPoint[33] = p4 - (p2 - p1)*0.5;
	pMouthPoint[35] = p3 + (p2 - p1)*0.5;
	pMouthPoint[34] = p4 + (p2 - p1)*0.5;


	pMouthPoint[36] = (pMouthPoint[32] + pMouthPoint[33])*0.5;
	pMouthPoint[37] = (pMouthPoint[34] + pMouthPoint[35])*0.5;


	Vector2 ScaleSize2(1.0 / m_nWidth, 1.0 / m_nHeight);
	int nVerts = m_MouthUV.size() / 2;
	for (int i = 0; i < nVerts; i++)
	{
		pMouthPoint[i] = pMouthPoint[i] * ScaleSize2 * 2.0 - Vector2(1.0, 1.0);
	}

	//freopen("point_mouthuv_test.txt", "w", stdout);
	//std::cout << nVerts *2 << std::endl;
	//Vector2 mouthP[50];
	//for (int i = 0; i < nVerts; i++) {
	//	mouthP[i].x = pMouthPoint[i].x * 0.5 + 0.5;
	//	mouthP[i].y = pMouthPoint[i].y * 0.5 + 0.5;
	//}

	//for (int i = 0; i < nVerts; i++) {
	//	float x = (mouthP[i].x*m_nWidth - 232) / 465;
	//	float y = (mouthP[i].y*m_nHeight - 620) / 248;
	//	if (x < 0) { x = 0.0; }
	//	if (x > 1.0) { x = 1.0; }
	//	if (y < 0) { y = 0.0; }
	//	if (y > 1.0) { y = 1.0; }
	//	std::cout << x << "f" << "," <<  y << "f" << ",";
	//}
	//std::cout << std::endl;
	//std::cout << std::endl;
	//fclose(stdout);

}

void FaceMeshManage::EyeInter(Vector2 * pFacePoint, Vector2 * pEyePoint)
{

	Vector2 pLeftEye[9];
	memcpy(pLeftEye, pFacePoint + 51, 9 * sizeof(Vector2));

	Vector2 pRightEye[9] = {
		pFacePoint[65],
		pFacePoint[64],
		pFacePoint[63],
		pFacePoint[62],
		pFacePoint[61],
		pFacePoint[68],
		pFacePoint[67],
		pFacePoint[66],
		pFacePoint[69],
	};

	float avgLength1 = 0.0;
	float avgLength2 = 0.0;
	for (int i = 2; i < 6; ++i) {
		avgLength1 += pLeftEye[i].distance(pLeftEye[i + 1]);
		avgLength2 += pRightEye[i].distance(pRightEye[i + 1]);
	}
	avgLength1 /= 4;
	avgLength2 /= 4;
	if (avgLength1 > avgLength2)
	{
		m_FixLength = avgLength1;
	}
	else
	{
		m_FixLength = avgLength2;
	}

	EyeInterLRNew(pLeftEye, pEyePoint, (Vector2*)m_EyeUV.data(), -1);// 

	Vector2 ScaleSize2(1.0 / m_nWidth, 1.0 / m_nHeight);
	int nVerts = m_EyeUV.size() / 2;//

	EyeInterLRNew(pRightEye, pEyePoint + nVerts / 2, (Vector2*)m_EyeUV.data() + nVerts / 2, 1);// 


	for (int i = 0; i < nVerts; i++)
	{
		pEyePoint[i] = pEyePoint[i] * ScaleSize2 * 2.0 - Vector2(1.0, 1.0);
	}
	//不裁剪取uv
	/*freopen("point_eye_uv.txt", "w", stdout);
	std::cout << nVerts *2<< std::endl;
	for (int i = 0; i < nVerts; i++) {
		std::cout << pEyePoint[i].x*0.5+0.5<< "f" << "," << pEyePoint[i].y*0.5+0.5 << "f" <<",";
	}

	//for (int i = 0; i < nVerts; i++) {
	//	std::cout << vRightEye[i].x*0.5 + 0.5 << "," << vRightEye[i].y*0.5 + 0.5 << ",";
	//}

	std::cout << std::endl;
	std::cout << std::endl;

	for (int i=0;i<m_EyeIndex.size();i++)
	{
		std::cout << m_EyeIndex[i] << ",";
	}

	for (int i = 0; i < m_EyeIndex.size(); i++)
	{
		std::cout << m_EyeIndex[i]  + nVerts/2 << ",";
	}
	fclose(stdout);*/
	//裁剪取uv
	/*freopen("point_eyeuv_test.txt", "w", stdout);
	std::cout << nVerts *2<< std::endl;
	Vector2 EyeP[188];
	for (int i = 0; i < nVerts; i++) {
		//std::cout << pEyePoint[i].x*0.5+0.5<< "f" << "," << pEyePoint[i].y*0.5+0.5 << "f" << ",";
		EyeP[i].x = pEyePoint[i].x * 0.5 + 0.5;
		EyeP[i].y = pEyePoint[i].y * 0.5 + 0.5;
	}

	for (int i = 0; i < nVerts; i++) {
		float x = (EyeP[i].x*m_nWidth - 232) / 465;
		float y = (EyeP[i].y*m_nHeight - 372) / 248;
		if (x < 0.0) { x = (float)0.0; }
		if (x > 1.0) { x = (float)1.0; }
		if (y < 0.0) { y = (float)0.0; }
		if (y > 1.0) { y = (float)1.0; }
		std::cout << x << "f" <<"," << y << "f" << ",";
	}

	std::cout << std::endl;
	std::cout << std::endl;

	//for (int i=0;i<m_EyeIndex.size();i++)
	//{
	//	std::cout << m_EyeIndex[i] << ",";
	//}

	//for (int i = 0; i < m_EyeIndex.size(); i++)
	//{
	//	std::cout << m_EyeIndex[i]  + nVerts/2 << ",";
	//}

	fclose(stdout);*/
}

void FaceMeshManage::EyeInterLR(Vector2 *pEyePointSrc, Vector2 *pEyePointDst, Vector2 *pEyeUV, float direct)
{
	for (int i = 0; i < 4; i++) {
		pEyePointDst[i * 2] = pEyePointSrc[i];
		pEyePointDst[i * 2 + 1] = (pEyePointSrc[i] + pEyePointSrc[i + 1])*0.5;
	}
	memcpy(pEyePointDst + 8, pEyePointSrc + 4, 4 * sizeof(Vector2));

	Vector2 *pEyePoint = pEyePointDst;
	float avgLength = 0.0;
	for (int i = 9; i < 12; ++i) {
		avgLength += pEyePoint[i].distance(pEyePoint[i - 1]);
	}
	avgLength /= 3;

	float length1 = direct * avgLength * 0.7;
	float length2 = direct * avgLength * 0.8;

	for (int i = 12; i < 20; i++)
	{
		pEyePoint[i] = CalVerticalWithLength(pEyePoint[i - 12], pEyePoint[i - 11], length1);
	}

	for (int i = 20; i < 23; i++)
	{
		pEyePoint[i] = CalVerticalWithLength(pEyePoint[i - 12], pEyePoint[i - 11], length2);
	}

	pEyePoint[23] = CalVerticalWithLength(pEyePoint[11], pEyePoint[0], length2);

	pEyePoint[24] = CalVerticalWithLength(pEyePoint[8], pEyePoint[7], -length2);

	pEyePoint[25] = pEyePoint[20] + pEyePoint[24] - pEyePoint[8];

	pEyePoint[26] = CalVerticalWithLength(pEyePoint[0], pEyePoint[11], -length2);
	pEyePoint[27] = pEyePoint[26] + pEyePoint[12] - pEyePoint[0];

	for (int i = 28; i < 35; i++)
	{
		pEyePoint[i] = CalVerticalWithLength(pEyePoint[i - 16], pEyePoint[i - 15], length2);
	}
	pEyePoint[35] = CalVerticalWithLength(pEyePoint[19], pEyePoint[24], length2);
	pEyePoint[36] = CalVerticalWithLength(pEyePoint[24], pEyePoint[19], -length2);


	{

		Vector2 *pSrcEyePoint = pEyeUV;
		Vector2 ScaleSize = Vector2(m_nWidth, m_nWidth * 800 / 650);
		Vector2 Translate = pSrcEyePoint[0] * ScaleSize - pEyePoint[0];
		//add
		Vector2 Translate_up_left = Vector2(-Translate.x, -Translate.y);
		Vector2 Translate_up_right = Vector2(Translate.x, -Translate.y);
		Vector2 Translate_down_left = Vector2(-Translate.x, Translate.y * 2);
		Vector2 Translate_down_right = Vector2(Translate.x, Translate.y * 2);

		Vector2 SrcPoint[4] = {
			pSrcEyePoint[37] * ScaleSize - Translate,
			pSrcEyePoint[38] * ScaleSize - Translate,
			pSrcEyePoint[39] * ScaleSize - Translate,
			pSrcEyePoint[40] * ScaleSize - Translate,
		};
		Vector2 DstPoint[5];

		Vector2 ControlSrc[6] =
		{
			pSrcEyePoint[27] * ScaleSize - Translate,
			pSrcEyePoint[24] * ScaleSize - Translate,
			pSrcEyePoint[10] * ScaleSize - Translate,
			pSrcEyePoint[20] * ScaleSize - Translate,
			pSrcEyePoint[22] * ScaleSize - Translate,
			pSrcEyePoint[26] * ScaleSize - Translate,
		};
		Vector2 ControlDst[6] =
		{
			pEyePoint[27],
			pEyePoint[24],
			pEyePoint[10],
			pEyePoint[20],
			pEyePoint[22],
			pEyePoint[26],
		};
		TransformVertexMls(SrcPoint, DstPoint, ControlSrc, ControlDst, 6, 4);
		memcpy(pEyePoint + 37, DstPoint, 4 * sizeof(Vector2));

		DstPoint[4] = DstPoint[0];
		Vector2 *pEye = pEyePoint + 41;
		for (int i = 0; i < 4; i++)
		{
			pEye[i * 3] = (DstPoint[i] + DstPoint[i + 1])*0.5;
			pEye[i * 3 + 1] = (DstPoint[i] + pEye[i * 3])*0.5;
			pEye[i * 3 + 2] = (DstPoint[i + 1] + pEye[i * 3])*0.5;
		}
	}
}

float isEyeOpened(Vector2 *pEyePoints, float param = 0.25)
{
	//�������´��ľ������ȡ��ֵ
	float Ist = 0;
	//ͨ���۽ǽǶȼ������۱���
	Vector2 up = (pEyePoints[3] - pEyePoints[4]).normalize();
	Vector2 down = (pEyePoints[5] - pEyePoints[4]).normalize();

	//�۽ǽǶ�
	float angle = 1.0 - up.dot(down);

	if (angle < param) {
		Ist = 0.0;
	}
	else
	{
		Ist = angle / param - 1.0;
	}
	return Ist;

}

void FaceMeshManage::EyeInterLRNew(Vector2 *pEyePointSrc, Vector2 *pEyePointDst, Vector2 *pEyeUV, float direct)//
{
	m_EyeAlpha = 1.0;
	//float eyeAlpha = isEyeOpened(pEyePointSrc, 0.05);
	float eyeAlpha = 1.0;
	if (eyeAlpha == 0.0)
	{
		pEyePointSrc[1] = pEyePointSrc[7];
		pEyePointSrc[2] = pEyePointSrc[6];
		pEyePointSrc[3] = pEyePointSrc[5];
	}

	Vector2 pOutput[13];
	//ƽ��ȡ��
	int outCnt = CMathUtils::SmoothLines2(pOutput, pEyePointSrc, 5, 0.5, 3);
	memcpy(pEyePointDst, pOutput, 12 * sizeof(Vector2));
	//13-23
	for (int i = 4; i < 8; i++) {
		pEyePointDst[i * 3] = pEyePointSrc[i];
		Vector2 offset;
		if (i == 7)
		{
			offset = pEyePointSrc[i] - pEyePointSrc[0];
		}
		else
		{
			offset = pEyePointSrc[i] - pEyePointSrc[i + 1];
		}
		pEyePointDst[i * 3 + 1] = pEyePointSrc[i] - offset * 0.33;
		pEyePointDst[i * 3 + 2] = pEyePointSrc[i] - offset * 0.66;
	}
	//������ȶ��ĵ�
	Vector2 StablePoint[7];
	StablePoint[0] = (pEyePointDst[0] + pEyePointSrc[4])*0.5;
	StablePoint[1] = (pEyePointDst[0] + StablePoint[0])*0.5;
	StablePoint[2] = (pEyePointDst[12] + StablePoint[1])*0.5;
	float length0 = StablePoint[0].distance(pEyePointSrc[4]);
	StablePoint[3] = Vector2(StablePoint[0].x, (StablePoint[0].y - length0));
	StablePoint[4] = Vector2(StablePoint[0].x, (StablePoint[0].y + length0));
	StablePoint[5] = (StablePoint[0] + StablePoint[3])*0.5;
	StablePoint[6] = (StablePoint[0] + StablePoint[4])*0.5;
	//�����ȶ�����
	Vector2 *pEyePoint = pEyePointDst;
	float length_corner = direct * m_FixLength * 4;

	//24-35
	/*//********ģ���۾��Ļ��ȣ��ڱ���ʱ�Ա��ֻ���*******
	//pEyePoint[24] = CalVerticalWithLength(pEyePoint[0], pEyePoint[1], length_corner * 0.1);
	//pEyePoint[25] = CalVerticalWithLength(pEyePoint[1], pEyePoint[2], length_corner * 0.12);
	//pEyePoint[26] = CalVerticalWithLength(pEyePoint[2], pEyePoint[3], length_corner * 0.16);
	//pEyePoint[27] = CalVerticalWithLength(pEyePoint[3], pEyePoint[4], length_corner * 0.17);
	//pEyePoint[28] = CalVerticalWithLength(pEyePoint[4], pEyePoint[5], length_corner * 0.19);
	//pEyePoint[29] = CalVerticalWithLength(pEyePoint[5], pEyePoint[6], length_corner * 0.23);
	//pEyePoint[30] = CalVerticalWithLength(pEyePoint[6], pEyePoint[7], length_corner * 0.24);
	//pEyePoint[31] = CalVerticalWithLength(pEyePoint[7], pEyePoint[8], length_corner * 0.21);
	//pEyePoint[32] = CalVerticalWithLength(pEyePoint[8], pEyePoint[9], length_corner * 0.17);
	//pEyePoint[33] = CalVerticalWithLength(pEyePoint[9], pEyePoint[10], length_corner * 0.13);
	//pEyePoint[34] = CalVerticalWithLength(pEyePoint[10], pEyePoint[11], length_corner * 0.11);
	//pEyePoint[35] = CalVerticalWithLength(pEyePoint[11], pEyePoint[12], length_corner * 0.08);*/

	Vector2 pInput1[4];
	Vector2 pOutput1[10];

	pEyePoint[24] = CalVerticalWithLength(pEyePoint[0], pEyePoint[1], length_corner * 0.02);//0.08ȡuv/0.02
	pInput1[0] = CalVerticalWithLength(pEyePoint[1], pEyePoint[2], length_corner * 0.04);//0.1ȡuv/0.04
	pInput1[1] = CalVerticalWithLength(pEyePoint[4], pEyePoint[5], length_corner * 0.15);//0.19ȡuv/0.15
	pInput1[2] = CalVerticalWithLength(pEyePoint[7], pEyePoint[8], length_corner * 0.17);//0.23ȡuv/0.17
	pInput1[3] = CalVerticalWithLength(pEyePoint[10], pEyePoint[11], length_corner * 0.05);//0.08ȡuv/0.05
	pEyePoint[35] = CalVerticalWithLength(pEyePoint[11], pEyePoint[12], length_corner * 0.04);//0.06ȡuv/0.04
	//平滑取点
	int outCnt1 = CMathUtils::SmoothLines2(pOutput1, pInput1, 4, 0.5, 3);
	if (eyeAlpha < 1.0)
	{
		m_EyeAlpha = eyeAlpha;
		//if (eyeAlpha == 0.0)
		//{
		//	pEyePoint[24] = CalVerticalWithLength(pEyePoint[0], pEyePoint[1], length_corner * 0.002);//0.002
		//	pInput1[0] = CalVerticalWithLength(pEyePoint[1], pEyePoint[2], length_corner * 0.002);//0.002
		//	pInput1[1] = CalVerticalWithLength(pEyePoint[4], pEyePoint[5], length_corner * 0.1);//0.1
		//	pInput1[2] = CalVerticalWithLength(pEyePoint[7], pEyePoint[8], length_corner * 0.2);//0.2
		//	pInput1[3] = CalVerticalWithLength(pEyePoint[10], pEyePoint[11], length_corner * 0.08);//0.08
		//	pEyePoint[35] = CalVerticalWithLength(pEyePoint[11], pEyePoint[12], length_corner * 0.04);//0.04
		//	//ƽ��ȡ��
		//	int outCnt1 = CMathUtils::SmoothLines2(pOutput1, pInput1, 4, 0.3, 3);

		//	pOutput1[1] = Vector2(pOutput1[1].x, pOutput1[1].y + abs(length_corner*0.03));
		//	pOutput1[2] = Vector2(pOutput1[2].x, pOutput1[2].y + abs(length_corner*0.03));
		//}
		//else
		//{
		//	pEyePoint[24] = CalVerticalWithLength(pEyePoint[0], pEyePoint[1], length_corner * 0.006);//0.002
		//	pInput1[0] = CalVerticalWithLength(pEyePoint[1], pEyePoint[2], length_corner * 0.002);//0.002
		//	pInput1[1] = CalVerticalWithLength(pEyePoint[4], pEyePoint[5], length_corner * 0.12);//0.1
		//	pInput1[2] = CalVerticalWithLength(pEyePoint[7], pEyePoint[8], length_corner * 0.22);//0.2
		//	pInput1[3] = CalVerticalWithLength(pEyePoint[10], pEyePoint[11], length_corner * 0.08);//0.08
		//	pEyePoint[35] = CalVerticalWithLength(pEyePoint[11], pEyePoint[12], length_corner * 0.04);//0.04
		//	//ƽ��ȡ��
		//	int outCnt1 = CMathUtils::SmoothLines2(pOutput1, pInput1, 4, 0.3, 3);

		//	pOutput1[1] = Vector2(pOutput1[1].x, pOutput1[1].y + abs(length_corner*0.03));
		//	pOutput1[2] = Vector2(pOutput1[2].x, pOutput1[2].y + abs(length_corner*0.03));
		//}



	}
	memcpy(pEyePoint + 25, pOutput1, 10 * sizeof(Vector2));


	/*for (int i = 24; i < 36; i++)
	{
		//pEyePoint[i] = CalVerticalWithLength(pEyePoint[i - 24], pEyePoint[i - 23], length_corner * 0.1);

	}*/
	//36-43
	Vector2 pInput2[2];
	pEyePoint[36] = CalVerticalWithLength(pEyePoint[12], StablePoint[0], length_corner * 0.2);
	//pEyePoint[37] = CalVerticalWithLength(pEyePoint[12], StablePoint[5], length_corner * 0.4);
	//pEyePoint[38] = CalVerticalWithLength(pEyePoint[12], StablePoint[3], length_corner * 0.7);
	//pEyePoint[40] = CalVerticalWithLength(pEyePoint[0], StablePoint[0], -length_corner * 0.3);
	//pEyePoint[39] = (pEyePoint[38] + pEyePoint[40])*0.5;
	pInput2[1] = CalVerticalWithLength(pEyePoint[12], StablePoint[5], length_corner * 0.4);
	pInput2[0] = CalVerticalWithLength(pEyePoint[0], StablePoint[0], -length_corner * 0.3);
	Vector2 pOutput2[4];
	//ƽ��ȡ��
	int outCnt2 = CMathUtils::SmoothLines2(pOutput2, pInput2, 2, 0.2, 3);
	pEyePoint[37] = pOutput2[3];
	pEyePoint[38] = pOutput2[2];
	pEyePoint[39] = pOutput2[1];
	pEyePoint[40] = pOutput2[0];

	pEyePoint[41] = CalVerticalWithLength(pEyePoint[36], StablePoint[5], -length_corner * 0.3);
	pEyePoint[42] = CalVerticalWithLength(pEyePoint[0], StablePoint[4], -length_corner * 0.3);
	pEyePoint[44] = CalVerticalWithLength(pEyePoint[40], StablePoint[0], length_corner*0.6);
	//pEyePoint[43] = (pEyePoint[42] + pEyePoint[44])*0.5;
	pEyePoint[43] = CalVerticalWithLength(pEyePoint[0], pEyePoint[40], -length_corner * 0.1);
	//45-55
	for (int i = 45; i < 55; i++)
	{
		if (i == 46)
		{
			CalVerticalWithLength(pEyePoint[i - 22], pEyePoint[i - 21], length_corner * 0.15);
		}
		pEyePoint[i] = CalVerticalWithLength(pEyePoint[i - 21], pEyePoint[i - 20], length_corner * 0.15);

	}

	/*if (eyeAlpha < 1.0)
	{
		if (eyeAlpha == 0.0)
		{
			pEyePoint[46] = Vector2(pEyePoint[46].x + direct * abs(length_corner*0.05), pEyePoint[46].y + abs(length_corner*0.1));
			pEyePoint[47] = Vector2(pEyePoint[47].x + direct * abs(length_corner*0.01), pEyePoint[47].y + abs(length_corner*0.1));
		}
		else
		{
			pEyePoint[46] = Vector2(pEyePoint[46].x + direct * abs(length_corner*0.05), pEyePoint[46].y + abs(length_corner*0.1));
			pEyePoint[47] = Vector2(pEyePoint[47].x + direct * abs(length_corner*0.005), pEyePoint[47].y + abs(length_corner*0.05));
		}
	}
	else
	{
		pEyePoint[46] = Vector2(pEyePoint[46].x - direct * abs(length_corner*0.05), pEyePoint[46].y + abs(length_corner*0.05));
		pEyePoint[47] = Vector2(pEyePoint[47].x - direct * abs(length_corner*0.04), pEyePoint[47].y + abs(length_corner*0.02));
	}*/

	pEyePoint[46] = Vector2(pEyePoint[46].x - direct * abs(length_corner*0.05), pEyePoint[46].y + abs(length_corner*0.05));
	pEyePoint[47] = Vector2(pEyePoint[47].x - direct * abs(length_corner*0.04), pEyePoint[47].y + abs(length_corner*0.02));

	pEyePoint[55] = CalVerticalWithLength(pEyePoint[35], pEyePoint[34], -length_corner * 0.1);
	//56-71
	pEyePoint[56] = CalVerticalWithLength(pEyePoint[12], StablePoint[3], -length_corner * 0.4);

	Vector2 pInput3[2];
	//pEyePoint[61] = CalVerticalWithLength(pEyePoint[0], StablePoint[5], length_corner * 1.0);
	//pEyePoint[62] = CalVerticalWithLength(pEyePoint[0], pEyePoint[12], length_corner * 1.0);
	//pEyePoint[64] = CalVerticalWithLength(pEyePoint[12], pEyePoint[42], -length_corner * 1.0);
	//pEyePoint[63] = (pEyePoint[62] + pEyePoint[64])*0.5;
	pInput3[0] = CalVerticalWithLength(pEyePoint[0], StablePoint[5], length_corner * 1.0);
	pInput3[1] = CalVerticalWithLength(pEyePoint[12], pEyePoint[42], -length_corner * 1.1);
	Vector2 pOutput3[4];
	//ƽ��ȡ��
	int outCnt3 = CMathUtils::SmoothLines2(pOutput3, pInput3, 2, 0.6, 3);
	memcpy(pEyePoint + 61, pOutput3, 4 * sizeof(Vector2));

	pEyePoint[65] = (pEyePoint[56] + pEyePoint[64])*0.5;
	pEyePoint[66] = CalVerticalWithLength(pEyePoint[56], StablePoint[5], -length_corner * 0.8);
	for (int i = 67; i < 70; i++)
	{
		pEyePoint[i] = CalVerticalWithLength(pEyePoint[i - 30], pEyePoint[i - 31], -length_corner * 0.3);
	}
	pEyePoint[71] = CalVerticalWithLength(pEyePoint[42], pEyePoint[40], -length_corner * 0.4);
	pEyePoint[70] = (pEyePoint[71] + pEyePoint[69])*0.5;
	//72-88
	pEyePoint[72] = CalVerticalWithLength(pEyePoint[68], pEyePoint[67], -length_corner * 0.3);
	pEyePoint[73] = CalVerticalWithLength(pEyePoint[68], pEyePoint[39], length_corner * 0.5);
	pEyePoint[75] = CalVerticalWithLength(pEyePoint[0], StablePoint[4], -length_corner * 1.2);
	pEyePoint[74] = (pEyePoint[75] + pEyePoint[73])*0.5;
	pEyePoint[77] = CalVerticalWithLength(pEyePoint[0], pEyePoint[74], -length_corner * 1.0);
	pEyePoint[76] = (pEyePoint[74] + pEyePoint[77])*0.5;
	pEyePoint[78] = CalVerticalWithLength(pEyePoint[0], StablePoint[3], length_corner * 1.5);
	pEyePoint[79] = CalVerticalWithLength(pEyePoint[0], StablePoint[3], length_corner * 1.0);
	pEyePoint[80] = CalVerticalWithLength(pEyePoint[78], pEyePoint[79], -length_corner * 0.4);
	pEyePoint[81] = CalVerticalWithLength(pEyePoint[76], pEyePoint[77], length_corner * 0.8);
	pEyePoint[83] = CalVerticalWithLength(pEyePoint[75], pEyePoint[39], -length_corner * 0.8);
	pEyePoint[82] = (pEyePoint[81] + pEyePoint[83])*0.5;
	pEyePoint[84] = CalVerticalWithLength(pEyePoint[68], pEyePoint[67], -length_corner * 0.6);
	pEyePoint[85] = CalVerticalWithLength(pEyePoint[36], pEyePoint[38], length_corner*0.6);
	pEyePoint[87] = (pEyePoint[77] + pEyePoint[79])*0.5;
	pEyePoint[88] = CalVerticalWithLength(pEyePoint[79], pEyePoint[61], -length_corner * 0.3);
	pEyePoint[86] = CalVerticalWithLength(pEyePoint[44], pEyePoint[0], -length_corner * 0.3);
	//89-93
	for (int i = 89; i < 91; i++)
	{
		pEyePoint[i] = CalVerticalWithLength(pEyePoint[i - 28], pEyePoint[i - 27], -length_corner * 0.25);
	}
	pEyePoint[91] = CalVerticalWithLength(pEyePoint[91 - 28], pEyePoint[91 - 27], -length_corner * 0.15);
	pEyePoint[92] = CalVerticalWithLength(pEyePoint[64], pEyePoint[63], length_corner * 0.2);
	pEyePoint[93] = CalVerticalWithLength(pEyePoint[62], pEyePoint[61], -length_corner * 0.3);
	//�ĸ��ǵ�
	pEyePoint[57] = CalVerticalWithLength(pEyePoint[80], pEyePoint[78], length_corner*0.8);
	pEyePoint[58] = CalVerticalWithLength(StablePoint[0], pEyePoint[79], -length_corner * 2.0);
	pEyePoint[59] = CalVerticalWithLength(pEyePoint[12], pEyePoint[39], length_corner * 2.0);
	pEyePoint[60] = pEyePoint[57] + pEyePoint[59] - pEyePoint[58];
}


Vector2 FaceMeshManage::CalVerticalWithLength(Vector2 pos1, Vector2 pos2, float length) {

	Vector2 vec = pos2 - pos1;

	Vector2 vecN = Vector2(-vec.y, vec.x).normalize();
	return pos1 + length * vecN;
}


void FaceMeshManage::TransformVertexMls(Vector2 * pSrcPoint, Vector2 * pDstPoint, Vector2 * pSrcControl, Vector2 * pDstControl, int nControlCount, int nPointCount)
{
	int nplength = nControlCount;
	MLS mls;
	//���Ƶ�p��q�����ά���飺
	//��һ��Ϊ������ֵ
	//�ڶ���Ϊ������ֵ
	const int row = 2;
	//ԭͼ���Ƶ�p
	float** tp = new float*[row];
	//ԭͼҪ�ƶ���Ŀ��Ƶ�q
	float** tq = new float*[row];

	int npoints = nPointCount;
	//ԭͼ��
	float** srcPoints = new float*[row];
	//�����
	float** desPoints = new float*[row];
	for (int i = 0; i < row; i++)
	{
		tp[i] = new float[nplength];
		tq[i] = new float[nplength];
		srcPoints[i] = new float[npoints];
		desPoints[i] = new float[npoints];
	}
	//���ÿ��Ƶ�
	for (int i = 0; i < nplength; i++)
	{
		tp[0][i] = pSrcControl[i].y;
		tp[1][i] = pSrcControl[i].x;

		tq[0][i] = pDstControl[i].y;
		tq[1][i] = pDstControl[i].x;
	}
	//����Ҫ�任�ĵ�
	for (int i = 0; i < nPointCount; i++)
	{
		srcPoints[0][i] = pSrcPoint[i].y;
		srcPoints[1][i] = pSrcPoint[i].x;
	}

	//����任
	mls.MLSD2DpointsDefAffine(srcPoints, desPoints, tp, tq, nplength, npoints);

	for (int i = 0; i < nPointCount; i++)
	{
		pDstPoint[i].y = desPoints[0][i];
		pDstPoint[i].x = desPoints[1][i];
	}
	SAFE_DELETE_ARRAY(srcPoints[0]);
	SAFE_DELETE_ARRAY(srcPoints[1]);
	SAFE_DELETE_ARRAY(srcPoints);
	SAFE_DELETE_ARRAY(desPoints[0]);
	SAFE_DELETE_ARRAY(desPoints[1]);
	SAFE_DELETE_ARRAY(desPoints);
	for (int i = 0; i < row; i++)
	{
		SAFE_DELETE_ARRAY(tq[i]);
		SAFE_DELETE_ARRAY(tp[i]);
	}
	SAFE_DELETE_ARRAY(tq);
	SAFE_DELETE_ARRAY(tp);

}
