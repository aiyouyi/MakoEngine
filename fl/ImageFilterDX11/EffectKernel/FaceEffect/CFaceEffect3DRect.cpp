#include "CFaceEffect3DRect.h"
#include "Toolbox/HeaderModelForCull.h"
#include "Toolbox/DXUtils/DXUtils.h"
#include "Toolbox\fileSystem.h"
#include "EffectKernel/ShaderProgramManager.h"


struct EffectConstantBuffer2
{
	Mat4 mWVP; //���Ͼ���
	Mat4 VP;
};


CFaceEffect3DRect::CFaceEffect3DRect()
{
	m_IndexBuffer = NULL;
	m_VerticeBuffer = NULL;
	m_2DInterFace = NULL;
	m_pDepthStateEnable = NULL;
	m_pDepthStateDisable = NULL;
	m_material = 0;
}


CFaceEffect3DRect::~CFaceEffect3DRect()
{
	Release();
}

void CFaceEffect3DRect::Release()
{
	SAFE_DELETE(m_material);
	SAFE_RELEASE_BUFFER(m_IndexBuffer);
	SAFE_DELETE(m_2DInterFace);
	SAFE_RELEASE_BUFFER(m_VerticeBuffer);
}

void * CFaceEffect3DRect::Clone()
{
	CFaceEffect3DRect* result = new CFaceEffect3DRect();
	*result = *this;
	return result;
}

bool CFaceEffect3DRect::ReadConfig(XMLNode & childNode, HZIP hZip, char * pFilePath)
{
	if (!childNode.isEmpty())
	{
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

		XMLNode nodeDrawable = childNode.getChildNode("drawable", 0);
		if (!nodeDrawable.isEmpty())
		{
			const char *szDrawableName = nodeDrawable.getAttribute("Material");
			SAFE_DELETE(m_material);
			if (szDrawableName != NULL)
			{
				int w, h;
				m_material = DXUtils::CreateTexFromZIP(hZip, szDrawableName, false);

			}
			const char *szScaleB = nodeDrawable.getAttribute("ScaleB");
			if (szScaleB !=NULL)
			{
				sscanf(szScaleB, "%f,%f,%f", &m_ScaleBottom.x, &m_ScaleBottom.y, &m_ScaleBottom.z);
			}
		}
		return true;
	}
	return false;
}

Vector3 minVector3(Vector3 &v1, Vector3 &v2)
{
	Vector3 v;
	v.x = min(v1.x, v2.x);
	v.y = min(v1.y, v2.y);
	v.z = min(v1.z, v2.z);
	return v;
}

Vector3 maxVector3(Vector3 &v1, Vector3 &v2)
{
	Vector3 v;
	v.x = max(v1.x, v2.x);
	v.y = max(v1.y, v2.y);
	v.z = max(v1.z, v2.z);
	return v;
}


bool CFaceEffect3DRect::Prepare()
{

// 	m_HeaderModel.m_arrIndex = g_headerModelForCullIndces;
// 	m_HeaderModel.m_arrPos = (vec3 *)g_headerModelForCullVertices;
// 	m_HeaderModel.m_nFaces = g_headerModelForCullFaceCount;
// 	m_HeaderModel.m_nVertex = g_headerModelForCullVerticesCount;
// 
// 	m_HeaderModel.updateGpuBuffer();

	Vector3 *pHeadPoint = (Vector3*)g_headerModelForCullVertices;
	for (int i=0;i<g_headerModelForCullVerticesCount;i++)
	{
		m_BoundingBox.maxPoint = maxVector3(m_BoundingBox.maxPoint, pHeadPoint[i]);
		m_BoundingBox.minPoint = minVector3(m_BoundingBox.minPoint, pHeadPoint[i]);
	}
	m_BoundingBox.centerPoint = (m_BoundingBox.maxPoint + m_BoundingBox.minPoint)*0.5;

	Vector3 Scale1(1.5, 1.2, 1.0);
	Vector3 Scale2(1.5, 1, 1.5);
	m_BoundingBox.maxPoint = m_BoundingBox.centerPoint + (m_BoundingBox.maxPoint - m_BoundingBox.centerPoint)*Scale1*0.8;
	m_BoundingBox.minPoint = m_BoundingBox.centerPoint + (m_BoundingBox.minPoint - m_BoundingBox.centerPoint)*Scale2*0.8;

	Vector3 scale3 = m_ScaleBottom;
	Front.LT = Vector3(m_BoundingBox.minPoint.x, m_BoundingBox.maxPoint.y, m_BoundingBox.maxPoint.z);
	Front.RT = Vector3(m_BoundingBox.maxPoint.x, m_BoundingBox.maxPoint.y, m_BoundingBox.maxPoint.z);
	Front.LB = Vector3(m_BoundingBox.minPoint.x, m_BoundingBox.minPoint.y, m_BoundingBox.maxPoint.z)*scale3;
	Front.RB = Vector3(m_BoundingBox.maxPoint.x, m_BoundingBox.minPoint.y, m_BoundingBox.maxPoint.z)*scale3;

	Back.LT = Vector3(m_BoundingBox.minPoint.x, m_BoundingBox.maxPoint.y, m_BoundingBox.minPoint.z);
	Back.RT = Vector3(m_BoundingBox.maxPoint.x, m_BoundingBox.maxPoint.y, m_BoundingBox.minPoint.z);
	Back.LB = Vector3(m_BoundingBox.minPoint.x, m_BoundingBox.minPoint.y, m_BoundingBox.minPoint.z)*scale3;
	Back.RB = Vector3(m_BoundingBox.maxPoint.x, m_BoundingBox.minPoint.y, m_BoundingBox.minPoint.z)*scale3;


	m_pSamplerLinear = DXUtils::SetSampler();
	CCVetexAttribute pAttribute[] =
	{
		{VERTEX_ATTRIB_POSITION, FLOAT_C3},
		{VERTEX_ATTRIB_NORMAL, FLOAT_C3},
		{VERTEX_ATTRIB_TEX_COORD, FLOAT_C2},
	};

	string path = m_resourcePath + "/Shader/face3dEffectRect.fx";
	m_pShader = ShaderProgramManager::GetInstance()->GetOrCreateShaderByPathAndAttribs(path, pAttribute, 3);


	m_2DInterFace = new mt3dface::MTFace2DInterFace();


	if (m_pDepthStateEnable == NULL)
	{
		m_pDepthStateEnable = ContextInst->fetchDepthStencilState(true, true);
	}
	if (m_pDepthStateDisable == NULL)
	{
		m_pDepthStateDisable = ContextInst->fetchDepthStencilState(false, false);
	}

	m_pConstantBuffer = DXUtils::CreateConstantBuffer(sizeof(EffectConstantBuffer2));
	return true;
}


void CFaceEffect3DRect::Render(BaseRenderParam & RenderParam)
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
	auto pDoubleBuffer = RenderParam.GetDoubleBuffer();

	pDoubleBuffer->SyncAToB();



	Vector2 *pSrcpoint118 = RenderParam.GetFacePoint(0, FACE_POINT_130);

	auto FaceMesh = m_2DInterFace->Get2DMesh((float*)pSrcpoint118, RenderParam.GetWidth(), RenderParam.GetHeight(), 0, mt3dface::MT_FACE_2D_BACKGROUND);
	Vector2 pDstPoint171[300];
	for (int i = 0; i < FaceMesh->nVertex; i++)
	{
		pDstPoint171[i].x = FaceMesh->pVertexs[i * 3]*0.5+0.5;
		pDstPoint171[i].y = FaceMesh->pVertexs[i * 3 + 1] * 0.5 + 0.5;
	}
	GeneratePoint(pDstPoint171);


	if (m_VerticeBuffer== NULL)
	{
		m_VerticeBuffer = DXUtils::CreateVertexBuffer((float*)m_HeadRectNormal.data(), m_HeadRectNormal.size(), 8);
	}
	else
	{
		DXUtils::UpdateVertexBuffer(m_VerticeBuffer, (float*)m_HeadRectNormal.data(), m_HeadRectNormal.size(), 8 * sizeof(float), 8 * sizeof(float));
	}

	int width = RenderParam.GetWidth();
	int height = RenderParam.GetHeight();
	

	EffectConstantBuffer2 matWVP;

	//memcpy(&matProjDX, &project[0][0], sizeof(XMMATRIX));
	//matProjDX = XMMatrixTranspose(matProjDX);
	FacePosInfo *pFaceInfo = RenderParam.GetFaceInfo(0);
	Mat4 matRotateX;
	Mat4 matRotateY;
	Mat4 matRotateZ;
	Mat4 matTranslate;
	Mat4 matRotateXYZ;
	Mat4::createRotationX((-pFaceInfo->pitch +10)/ 180.0f*PI, &matRotateX);
	Mat4::createRotationY(-pFaceInfo->yaw / 180.0f*PI, &matRotateY);
	Mat4::createRotationZ(pFaceInfo->roll / 180.0f*PI, &matRotateZ);

	Mat4::multiply(matRotateX, matRotateY, &matRotateXYZ);
	Mat4::multiply(matRotateXYZ, matRotateZ, &matRotateXYZ);

	matWVP.VP = matRotateXYZ;


	Mat4::createTranslation(Vec3(pFaceInfo->x, pFaceInfo->y, -pFaceInfo->z), &matTranslate);
	Mat4::multiply(matTranslate, matRotateXYZ, &matRotateXYZ);

	ContextInst->setCullMode(D3D11_CULL_FRONT);
	pDoubleBuffer->BindFBOA();
	//set roi viewport
	D3D11_VIEWPORT vp;
	vp.Width = (FLOAT)pFaceInfo->pFaceRect.width;
	vp.Height = (FLOAT)pFaceInfo->pFaceRect.height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = pFaceInfo->pFaceRect.x;
	vp.TopLeftY = pFaceInfo->pFaceRect.y;
	DeviceContextPtr->RSSetViewports(1, &vp);

	DeviceContextPtr->ClearDepthStencilView(pDoubleBuffer->GetFBOA()->getDepthStencilView(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	DeviceContextPtr->OMSetDepthStencilState(m_pDepthStateEnable, 0);
	m_pShader->useShader();

	Mat4 matProj;
	
	float fAngle = atan(0.5) * 2;
	if (pFaceInfo->pFaceRect.height > pFaceInfo->pFaceRect.width)
	{
		fAngle = atan(0.5*pFaceInfo->pFaceRect.height / pFaceInfo->pFaceRect.width) * 2;
	}
	XMMATRIX matProjDX = XMMatrixPerspectiveFovRH(fAngle, pFaceInfo->pFaceRect.width*1.f / pFaceInfo->pFaceRect.height, 1, 2000);
	matProjDX = XMMatrixTranspose(matProjDX);

	memcpy(&matProj, &matProjDX, sizeof(XMMATRIX));
	Mat4::multiply(matProj, matRotateXYZ, &matRotateXYZ);

	matWVP.mWVP = matRotateXYZ;


	//���þ����任
	DeviceContextPtr->UpdateSubresource(m_pConstantBuffer, 0, NULL, &matWVP, 0, 0);
	DeviceContextPtr->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
	auto pSrcShaderView = pDoubleBuffer->GetFBOTextureB()->getTexShaderView();
	DeviceContextPtr->PSSetShaderResources(0, 1, &pSrcShaderView);
	pSrcShaderView = m_material->getTexShaderView();
	DeviceContextPtr->PSSetShaderResources(1, 1, &pSrcShaderView);
	DeviceContextPtr->PSSetSamplers(0, 1, &m_pSamplerLinear);

	//���ö�������
	unsigned int nStride = sizeof(HeadRectVertex);
	unsigned int nOffset = 0;
	DeviceContextPtr->IASetVertexBuffers(0, 1, &m_VerticeBuffer, &nStride, &nOffset);
	DeviceContextPtr->IASetIndexBuffer(m_IndexBuffer, DXGI_FORMAT_R16_UINT, 0);
	DeviceContextPtr->DrawIndexed(108, 0, 0);

	DeviceContextPtr->OMSetDepthStencilState(m_pDepthStateDisable, 0);
	ContextInst->setCullMode(D3D11_CULL_NONE);
}

void CFaceEffect3DRect::GeneratePoint(Vector2 *pFacePoint)
{
	if (m_HeadRect.size()==0)
	{
		HeadRectVertex VertexTmp;
		VertexTmp.Vertex = Front.LT;
		m_HeadRect.push_back(VertexTmp); //0

		VertexTmp.Vertex = Front.RT;
		m_HeadRect.push_back(VertexTmp);//1

		VertexTmp.Vertex = Front.LT + (Front.LB - Front.LT)*0.4;
		m_HeadRect.push_back(VertexTmp);//2

		VertexTmp.Vertex = Front.LT + (Front.LB - Front.LT)*0.7;
		m_HeadRect.push_back(VertexTmp);//3;

		VertexTmp.Vertex = Front.LB;
		m_HeadRect.push_back(VertexTmp);//4;

		VertexTmp.Vertex = Front.RB;
		m_HeadRect.push_back(VertexTmp);//5;

		VertexTmp.Vertex = Front.RT + (Front.RB - Front.RT)*0.7;
		m_HeadRect.push_back(VertexTmp);//6;

		VertexTmp.Vertex = Front.RT + (Front.RB - Front.RT)*0.4;
		m_HeadRect.push_back(VertexTmp);//7

		VertexTmp.Vertex = (m_HeadRect[2].Vertex + m_HeadRect[7].Vertex)*0.5;
		m_HeadRect.push_back(VertexTmp);//8

		VertexTmp.Vertex = Back.LT;
		m_HeadRect.push_back(VertexTmp);//9;

		VertexTmp.Vertex = Back.LB;
		m_HeadRect.push_back(VertexTmp);//10;

		VertexTmp.Vertex = Back.RT;
		m_HeadRect.push_back(VertexTmp);//11;

		VertexTmp.Vertex = Back.RB;
		m_HeadRect.push_back(VertexTmp);//12;

		VertexTmp.Vertex = Back.LT + (Back.LB - Back.LT)*0.4;
		m_HeadRect.push_back(VertexTmp);//13

		VertexTmp.Vertex = Back.LT + (Back.LB - Back.LT)*0.7;
		m_HeadRect.push_back(VertexTmp);//14;

		VertexTmp.Vertex = Back.RT + (Back.RB - Back.RT)*0.7;
		m_HeadRect.push_back(VertexTmp);//15;

		VertexTmp.Vertex = Back.RT + (Back.RB - Back.RT)*0.4;
		m_HeadRect.push_back(VertexTmp);//16

		Vector3 NoseCenter = (m_HeadRect[2].Vertex + m_HeadRect[4].Vertex + m_HeadRect[5].Vertex + m_HeadRect[7].Vertex)*0.25f;
		VertexTmp.Vertex = NoseCenter + (m_HeadRect[2].Vertex - m_HeadRect[8].Vertex)*0.33*0.9;
		m_HeadRect.push_back(VertexTmp);//17

		VertexTmp.Vertex = NoseCenter + (m_HeadRect[7].Vertex - m_HeadRect[8].Vertex)*0.33*0.9;
		m_HeadRect.push_back(VertexTmp);//18

		Vector3 MouthCenter = (m_HeadRect[3].Vertex + m_HeadRect[4].Vertex + m_HeadRect[5].Vertex + m_HeadRect[6].Vertex)*0.25f;

		VertexTmp.Vertex = MouthCenter + (m_HeadRect[2].Vertex - m_HeadRect[8].Vertex)*0.5*0.9;
		VertexTmp.Vertex.y = (MouthCenter.y + m_HeadRect[17].Vertex.y)*0.5;
		m_HeadRect.push_back(VertexTmp);//19

		VertexTmp.Vertex = MouthCenter + (m_HeadRect[7].Vertex - m_HeadRect[8].Vertex)*0.5*0.9;
		VertexTmp.Vertex.y = (MouthCenter.y + m_HeadRect[18].Vertex.y)*0.5;
		m_HeadRect.push_back(VertexTmp);//20

		VertexTmp.Vertex = MouthCenter + (m_HeadRect[2].Vertex - m_HeadRect[8].Vertex)*0.5*0.9;
		m_HeadRect.push_back(VertexTmp);//21

		VertexTmp.Vertex = MouthCenter + (m_HeadRect[7].Vertex - m_HeadRect[8].Vertex)*0.5*0.9;
		m_HeadRect.push_back(VertexTmp);//22
	}

	m_HeadRect[0].UV = pFacePoint[108];
	m_HeadRect[1].UV = pFacePoint[111];
	m_HeadRect[2].UV = pFacePoint[127];
	m_HeadRect[3].UV = pFacePoint[139];
	m_HeadRect[4].UV = pFacePoint[15]+ (pFacePoint[247]- pFacePoint[15])*0.2;
	m_HeadRect[5].UV = pFacePoint[17] + (pFacePoint[253] - pFacePoint[17])*0.2;;
	m_HeadRect[6].UV = pFacePoint[160];
	m_HeadRect[7].UV = pFacePoint[128];
	m_HeadRect[8].UV = (m_HeadRect[2].UV + m_HeadRect[7].UV)*0.5;
	m_HeadRect[9].UV.x = (pFacePoint[108].x + pFacePoint[237].x)*0.5;
	m_HeadRect[9].UV.y = (pFacePoint[108].y + pFacePoint[236].y)*0.5;
	m_HeadRect[10].UV = pFacePoint[16];
	m_HeadRect[11].UV.x = (pFacePoint[111].x + pFacePoint[240].x)*0.5;
	m_HeadRect[11].UV.y = (pFacePoint[111].y + pFacePoint[239].y)*0.5;
	m_HeadRect[12].UV = pFacePoint[16];
	m_HeadRect[13].UV = (pFacePoint[0] + pFacePoint[242])*0.5;
	m_HeadRect[14].UV = pFacePoint[7];
	m_HeadRect[15].UV = pFacePoint[25];
	m_HeadRect[16].UV = (pFacePoint[32] + pFacePoint[258])*0.5;

	m_HeadRect[17].UV = pFacePoint[77];
	m_HeadRect[18].UV = pFacePoint[83];
	m_HeadRect[19].UV = pFacePoint[148];
	m_HeadRect[20].UV = pFacePoint[169];
	m_HeadRect[21].UV = pFacePoint[230];// +(pFacePoint[86] - pFacePoint[89]);
	m_HeadRect[22].UV = pFacePoint[234];// +(pFacePoint[92] - pFacePoint[89]);

	Vector3 MouthCenter = (m_HeadRect[3].Vertex + m_HeadRect[4].Vertex + m_HeadRect[5].Vertex + m_HeadRect[6].Vertex)*0.25f;
	//根据张嘴程度修正下嘴唇
	float rate = pFacePoint[104].distance(pFacePoint[100])/pFacePoint[95].distance(pFacePoint[100]);

	float ratex = 0.66*(pFacePoint[230].x - pFacePoint[148].x) / (pFacePoint[169].x - pFacePoint[148].x);
	m_HeadRect[21].Vertex.x = m_HeadRect[19].Vertex.x + (m_HeadRect[20].Vertex.x - m_HeadRect[19].Vertex.x)*ratex;
	m_HeadRect[22].Vertex.x = m_HeadRect[20].Vertex.x + (m_HeadRect[19].Vertex.x - m_HeadRect[20].Vertex.x)*ratex;

	m_HeadRect[21].Vertex.y = MouthCenter.y + (m_HeadRect[4].Vertex.y - MouthCenter.y)*rate;
	m_HeadRect[22].Vertex.y = MouthCenter.y + (m_HeadRect[5].Vertex.y - MouthCenter.y)*rate;
	
	unsigned short pIndex[] = {
	0,11,1,11,0,9,1,16,7,16,1,11,22,4,21,4,22,5,0,1,8,0,8,2,14,2,3,2,14,13,0,2,13,7,18,8,18,7,6,4,14,3,14,4,10,3,2,17,4,3,19,6,7,15,4,12,10,12,4,5,15,7,16,17,2,8,5,6,15,6,5,22,19,3,17,5,15,12,1,7,8,9,0,13,17,8,18,20,6,22,6,20,18,17,18,19,19,18,20,4,19,21,19,20,21,21,20,22,
	//22,4,21,4,22,5,0,1,8,0,8,2,7,18,8,18,7,6,3,2,17,4,3,19,17,2,8,6,5,22,19,3,17,1,7,8,17,8,18,20,6,22,6,20,18,17,18,19,19,18,20,4,19,21,19,20,21,21,20,22
	};

	m_HeadRectNormal.clear();
	for (int i=0;i<108;i+=3)
	{
		Vector3 v1 = m_HeadRect[pIndex[i]].Vertex;
		Vector3 v2 = m_HeadRect[pIndex[i+1]].Vertex;
		Vector3 v3 = m_HeadRect[pIndex[i+2]].Vertex;

		Vector3 N = (v2 - v1).cross(v2 - v3);
		N = N.normalize();
		HeadRectVertex VertexTmp = m_HeadRect[pIndex[i]];
		VertexTmp.Normal = N;
		m_HeadRectNormal.push_back(VertexTmp);
		VertexTmp = m_HeadRect[pIndex[i+1]];
		VertexTmp.Normal = N;
		m_HeadRectNormal.push_back(VertexTmp);
		VertexTmp = m_HeadRect[pIndex[i+2]];
		VertexTmp.Normal = N;
		m_HeadRectNormal.push_back(VertexTmp);
		
		pIndex[i] = i;
		pIndex[i+1] = i+1;
		pIndex[i+2] = i+2;
		
	}
	if (m_IndexBuffer == NULL)
	{
		m_IndexBuffer = DXUtils::CreateIndexBuffer(pIndex, 36);
	}
}

