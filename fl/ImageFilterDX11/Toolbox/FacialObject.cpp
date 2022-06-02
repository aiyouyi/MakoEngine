#include "FacialObject.h"


void FOActionCallback(int event, const void *lpParam)
{
	if (lpParam == NULL)return;
	FacialCallbackInfo *pEvent = (FacialCallbackInfo *)lpParam;
	pEvent->event = event;
	if (pEvent->pTarget != NULL)
	{
		pEvent->pTarget->postEvent(pEvent);
	}
}

FacialObject* FacialObject::createFromZip(const std::string &file, HZIP hZip, const std::string &zip)
{
	Sprite3DResourceLoaderFromZip zipLoader(hZip, zip);

	FacialObject *object = new (std::nothrow) FacialObject();
	if (object && object->initWithFile(file, &zipLoader))
	{
		object->autorelease();
		return object;
	}
	CC_SAFE_DELETE(object);
	return nullptr;
}

FacialObject::~FacialObject()
{
	vector<FacialCallbackInfo *>::iterator it = m_listCallBack.begin();
	while (it != m_listCallBack.end())
	{
		FacialCallbackInfo *info = (*it);
		if (info != NULL)
		{
			info->release();
		}

		++it;
	}

	m_listCallBack.clear();
}

void FacialObject::setNeckBone(const char *szNeckBone, const char *szNeckBone2)
{
	if (szNeckBone != NULL)
	{
		m_szNeckBone = szNeckBone;
	}

	if (szNeckBone2 != NULL)
	{
		m_szNeckBone2 = szNeckBone2;
	}
}

void FacialObject::updateFacialInfo(float *arrExpAlpha)
{
	//return;
// 	int flip_idx[51] = {
// 		1, 0, 3, 2, 4, 6, 5, 8, 7, 10, 9, 12, 11, 13, 15, 14, 17, 16, 19, 18, 20, 21, 22, 23, 25, 24, 27, 26, 29, 28, 31, 30, 33, 32, 34, 36, 35, 37, 38, 40, 39, 42, 41, 43, 44, 46, 45, 48, 47, 50, 49,
// 	};
// 
// 
// 	float arrExpAlphaT[51];
// 	for (int i = 0; i < 51; ++i)
// 	{
// 		arrExpAlphaT[i] = arrExpAlpha[flip_idx[i]] * 100;
// 	}
// 	for (int i = 0; i < _meshVertexDatas.size(); ++i)
// 	{
// 		_meshVertexDatas.at(i)->updateVertexBuffer(arrExpAlphaT, 51);
// 	}
// 
// 	float fAlphaTeeth = arrExpAlphaT[23];
// 	for (int i = 0; i < _meshVertexDatas.size(); ++i)
// 	{
// 		_meshVertexDatas.at(i)->updateVertexBuffer(&fAlphaTeeth, 1);
// 	}

		//return;
	int flip_idx[51] = {
		1, 0, 3, 2, 4, 6, 5, 8, 7, 10, 9, 12, 11, 13, 15, 14, 17, 16, 19, 18, 20, 21, 22, 23, 25, 24, 27, 26, 29, 28, 31, 30, 33, 32, 34, 36, 35, 37, 38, 40, 39, 42, 41, 43, 44, 46, 45, 48, 47, 50, 49,
	};


	int mapIndex[] =
	{
		1,1,
		2,2		,
		3,3   	,
		4,4		,
		5,27	,
		6,28	,
		7,29	,
		8,30	,
		9,25	,
		10,26	,
		11,31	,
		12,32	,
		13,33	,
		14,34	,
		15,6	,
		16,7	,
		17,5	,
		18,48	,
		19,49	,
		20,35	,
		21,36	,
		22,24	,
		23,37	,
		24,40	,
		25,41	,
		26,10	,
		27,11	,
		28,8	,
		29,9	,
		30,42	,
		31,43	,
		32,12	,
		33,13	,
		34,45	,
		35,44	,
		36,15	,
		36,16	,
		37,17	,
		37,18	,
		39,39	,
		40,23	,
		43,19	,
		43,20	,
		44,14	,
		45,50	,
		46,51	,
	};


// 	int mapIndex[] =
// 	{
// 		1,1,
// 		2,2		,
// 		9,25	,
// 		10,26	,
// 		15,6	,
// 		16,7	,
// 		17,5	,
// 		18,48	,
// 		19,49	,
// 		20,35	,
// 		21,36	,
// 		22,24	,
// 		23,37	,
// 		24,40	,
// 		25,41	,
// 		26,10	,
// 		27,11	,
// 		28,8	,
// 		29,9	,
// 		32,12	,
// 		33,13	,
// 		34,45	,
// 		35,44	,
// 		36,15   ,
// 		36,16,
// 		37,17,
// 		37,18,
// 		39,39	,
// 		40,23	,
// 		43,19	,
// 		43,20	,
// 		44,14	,
// 		45,50	,
// 		46,51	,
// 	};
	float arrExpAlphaT[51];
	memset(arrExpAlphaT, 0, sizeof(float) * 51);
	for (int i=0;i<46;i++)
	{
		arrExpAlphaT[mapIndex[2 * i + 1] - 1] = arrExpAlpha[mapIndex[2 * i]] * 100;
	}

	
// 	for (int i = 0; i < 51; ++i)
// 	{
// 		arrExpAlphaT[i] = arrExpAlpha[flip_idx[i]] * 100;
// 	}
	for (int i = 0; i < _meshVertexDatas.size(); ++i)
	{
		_meshVertexDatas.at(i)->updateVertexBuffer(arrExpAlphaT, 51);
	}

	float fAlphaTeeth = arrExpAlphaT[23];
	for (int i = 0; i < _meshVertexDatas.size(); ++i)
	{
		_meshVertexDatas.at(i)->updateVertexBuffer(&fAlphaTeeth, 1);
	}


}

void FacialObject::updateNeckRotate(float *arrQ)
{
	Bone3D *boneNeck = _skeleton->getBoneByName(m_szNeckBone);
	if (boneNeck != NULL)
	{
		Quaternion qy(Vec3(1, 0, 0), -arrQ[1]/2 * 1 / 180.0*3.14f); //y
		Quaternion qz(Vec3(0, 1, 0), arrQ[2]/2 * 1 / 180.0*3.14f); //z
		Quaternion qx(Vec3(0, 0, 1), -(arrQ[0]/2 * 1 ) / 180.0*3.14f); //x
		Quaternion qRes;
		Quaternion::multiply(qy, qx, &qRes);
		Quaternion::multiply(qz, qRes, &qx);
		boneNeck->setAnimationValue(NULL, (float *)&qx, NULL, NULL, m_fNeckContrlWeight);
	}

	boneNeck = _skeleton->getBoneByName(m_szNeckBone2);
	if (boneNeck != NULL)
	{
		Quaternion qy(Vec3(1, 0, 0), -arrQ[1]/2 * 1 / 180.0*3.14f); //y
		Quaternion qz(Vec3(0, 1, 0), arrQ[2]/2 * 1 / 180.0*3.14f); //z
		Quaternion qx(Vec3(0, 0, 1), -(arrQ[0]/2 * 1) / 180.0*3.14f); //x
		Quaternion qRes;
		Quaternion::multiply(qy, qx, &qRes);
		Quaternion::multiply(qz, qRes, &qx);
		boneNeck->setAnimationValue(NULL, (float *)&qx, NULL, NULL, m_fNeckContrlWeight);
	}


	
}


void EyeRotationRetargetingAux(Vector2* eye_pose, Vector2 *pFacePoint)
{
	// 2D FA points
	Vector2 pPupilLeft, pPupilRight;
	pPupilLeft = pFacePoint[59];
	pPupilRight = pFacePoint[69];

	Vector2 pEyeLeft, pEyeRight;
	pEyeLeft = pFacePoint[60];
	pEyeRight = pFacePoint[70];

	// Transfer percentage
	Vector2 t1, t2;
	float ex1, ey1, ex2, ey2;

	ex1 = (pFacePoint[55] - pFacePoint[51]).length();
	ey1 = (pFacePoint[57] - pFacePoint[53]).length();
	ex2 = (pFacePoint[65] - pFacePoint[61]).length();
	ey2 = (pFacePoint[67] - pFacePoint[63]).length();

	t1 = pPupilLeft - pEyeLeft;
	t2 = pPupilRight - pEyeRight;

	eye_pose[0].x = t1.x / (ex1 * 0.5f);
	eye_pose[0].y = t1.y / (ey1 * 0.5f);
	eye_pose[1].x = t2.x / (ex2 * 0.5f);
	eye_pose[1].y = t2.y / (ey2 * 0.5f);
}

void EyePoseRetargeting(Vector2 *pFacePoint, Vector3 *vEyeAngle)
{
	Vector2 vEyeRotationScale = Vector2(0.1, 1.0);
	float maxAngle = 65.0f/ 57.296;

	float m_pEyeRotMaxAngle[4] = {
		maxAngle,maxAngle,maxAngle,maxAngle
	};


	Vector2 vEyePose[2];
	EyeRotationRetargetingAux(vEyePose,pFacePoint); // Calculate current eye pose.

	Vector2 r1 = vEyePose[0];
	Vector2 r2 = vEyePose[1];
	Vector3 vEyeAngle_L, vEyeAngle_R;

	// Left eye rotation
	vEyeAngle_L.x = r1.y >= 0 ? m_pEyeRotMaxAngle[0] * r1.y : m_pEyeRotMaxAngle[1] * r1.y;
	vEyeAngle_L.y = r1.x >= 0 ? m_pEyeRotMaxAngle[2] * r1.x : m_pEyeRotMaxAngle[3] * r1.x;
	vEyeAngle_L.x = vEyeAngle_L.x * vEyeRotationScale.x;
	vEyeAngle_L.y = vEyeAngle_L.y * vEyeRotationScale.y;

	// Right eye rotation
	vEyeAngle_R.x = r2.y >= 0 ? m_pEyeRotMaxAngle[0] * r2.y : m_pEyeRotMaxAngle[1] * r2.y;
	vEyeAngle_R.y = r2.x >= 0 ? m_pEyeRotMaxAngle[2] * r2.x : m_pEyeRotMaxAngle[3] * r2.x;
	vEyeAngle_R.x = vEyeAngle_R.x * vEyeRotationScale.x;
	vEyeAngle_R.y = vEyeAngle_R.y * vEyeRotationScale.y;

	vEyeAngle[0] = vEyeAngle_L;
	vEyeAngle[1] = vEyeAngle_R;
// 
// 	vEyeAngle_L *= m_fEyeBlinkRatio_R;
// 	vEyeAngle_R *= m_fEyeBlinkRatio_R;
}

void FacialObject::updateEyeRotate(Vector2 *pFacePoint, float *eyeBlinkParam)
{

	Vector3 EyeRotate[2];
	EyePoseRetargeting(pFacePoint, EyeRotate);
	EyeRotate[0] *= (1.0 - eyeBlinkParam[0]);
	EyeRotate[1] *= (1.0 - eyeBlinkParam[1]);

	Bone3D *boneEye = _skeleton->getBoneByName("CATRigHub003_Eye_R01");
	if (boneEye != NULL)
	{
		Quaternion qy(Vec3(1, 0, 0), EyeRotate[0].y ); //y
		Quaternion qz(Vec3(0, 1, 0), EyeRotate[0].z ); //z
		Quaternion qx(Vec3(0, 0, 1), EyeRotate[0].x ); //x
		Quaternion qRes;
		Quaternion::multiply(qy, qx, &qRes);
		Quaternion::multiply(qz, qRes, &qx);
		boneEye->setAnimationValue(NULL, (float *)&qx, NULL, NULL, m_fNeckContrlWeight);
	}
	boneEye = _skeleton->getBoneByName("CATRigHub003_Eye_L01");
	if (boneEye != NULL)
	{
		Quaternion qy(Vec3(1, 0, 0), EyeRotate[1].y ); //y
		Quaternion qz(Vec3(0, 1, 0), EyeRotate[1].z ); //z
		Quaternion qx(Vec3(0, 0, 1), EyeRotate[1].x ); //x
		Quaternion qRes;
		Quaternion::multiply(qy, qx, &qRes);
		Quaternion::multiply(qz, qRes, &qx);
		boneEye->setAnimationValue(NULL, (float *)&qx, NULL, NULL, m_fNeckContrlWeight);
	}
}

void FacialObject::playExt(const char *szID, ActionUserCallback pCallBack, void *pUserInfo)
{
	FacialCallbackInfo *pEvent = NULL;
	if (pCallBack != NULL)
	{
		pEvent = new FacialCallbackInfo();
		pEvent->setInfo(this, szID, 0, pCallBack, pUserInfo);
		pEvent->autorelease();
	}
	play(szID, FOActionCallback, pEvent);
	if (strcmp(szID, "idle") == 0)
	{
		m_autoPlay = false;
	}
	else
	{
		m_autoPlay = true;
	}
}

void FacialObject::updateBlendWeights(int meshIndex, const map<int, float> &mapWeights)
{
	if (m_autoPlay)
	{
		__super::updateBlendWeights(meshIndex, mapWeights);
	}
}

//m_fNeckContrlWeight = 0.0f;
//m_fWeight = 0.0f;
void FacialObject::update(float ftime)
{
	__super::update(ftime);
	if (m_autoPlay)
	{
		if (m_fWeight > 0.0f)
		{
			m_fWeight -= ftime*2.0f;
		}
		if (m_fWeight < 0.0f)
		{
			m_fWeight = 0.0f;
		}
		
		m_fNeckContrlWeight = 1.0f*m_fWeight / (1.0f - m_fWeight);
	}
	else
	{
		if (m_fWeight < 0.99f)
		{
			m_fWeight += ftime*4.0f;
		}
		if (m_fWeight > 0.99f)
		{
			m_fWeight = 0.99f;
		}

		m_fNeckContrlWeight = 1.0f*m_fWeight / (1.0f - m_fWeight);
	}

	vector<FacialCallbackInfo *>::iterator it = m_listCallBack.begin();
	while (it != m_listCallBack.end())
	{
		FacialCallbackInfo *info = (*it);
		if (info != NULL)
		{
			if (info->pCallBack != NULL)
			{
				(*(info->pCallBack))(info->event, info->szAnimationName.c_str(), info->pUserInfo);
			}

			info->release();
		}
		
		++it;
	}

	m_listCallBack.clear();
}

void FacialObject::postEvent(FacialCallbackInfo *pEvent)
{
	if (pEvent != NULL)
	{
		FacialCallbackInfo *pEventT = new FacialCallbackInfo();
		pEventT->autorelease();
		pEventT->retain();
		pEventT->setInfo(this, pEvent->szAnimationName, pEvent->event, pEvent->pCallBack, pEvent->pUserInfo);
		m_listCallBack.push_back(pEventT);
	}
}