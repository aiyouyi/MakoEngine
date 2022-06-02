#include "CFaceEffect3DModel.h"
#include "Toolbox/HeaderModelForCull.h"
#include "Toolbox/DXUtils/DXUtils.h"
#include "Toolbox\fileSystem.h"
#include "EffectKernel/ShaderProgramManager.h"
//#include "opencv2/opencv.hpp"

struct EffectConstantBuffer
{
	Mat4 mWVP; //���Ͼ���
	Mat4 mNormal;
	XMVECTOR mColor;
};

struct HeaderCullConstantBuffer
{
	Mat4 mWVP; //���Ͼ���
};
CFaceEffect3DModel::CFaceEffect3DModel()
{
	m_nEffectDuring = -1;
	m_nAniLoopTime = -1;
	m_nStartTime = -1;
	m_nLoopStartTime = -1;
	m_pSamplerLinear = NULL;
	//��Ⱦģ����Ϣ
	m_pShaderForHeaderCull = NULL;

	m_pConstantBuffer = NULL;
	m_pConstantBufferForHeaderCull = NULL;

	//�����Լ�buffer����
	m_pBSEnable = NULL;
	m_pBSWriteDisable = NULL;
	m_pBSDisable = NULL;
	m_pBSDisableWriteDisable = NULL;

	//���Ȳ���
	m_pDepthStateEnable = NULL;
	m_pDepthStateDisable = NULL;
	m_pDepthStateEnableWriteDisable = NULL;
	m_vEffectModel.clear();

}


CFaceEffect3DModel::~CFaceEffect3DModel()
{
	Release();
}

void CFaceEffect3DModel::Release()
{
	m_HeaderModel.destory();

	SAFE_RELEASE_BUFFER(m_pConstantBuffer);
	SAFE_RELEASE_BUFFER(m_pConstantBufferForHeaderCull);

	SAFE_RELEASE_BUFFER(m_pBSEnable);
	SAFE_RELEASE_BUFFER(m_pBSWriteDisable);
	SAFE_RELEASE_BUFFER(m_pBSDisable);
	SAFE_RELEASE_BUFFER(m_pBSDisableWriteDisable);

	SAFE_RELEASE_BUFFER(m_pDepthStateEnable);
	SAFE_RELEASE_BUFFER(m_pDepthStateDisable);
	SAFE_RELEASE_BUFFER(m_pDepthStateEnableWriteDisable);
	SAFE_RELEASE_BUFFER(m_pSamplerLinear);

	SAFE_DELETE(m_MatCapNormal);
	SAFE_DELETE(m_MatCap);
	for (int  i = 0; i < m_vEffectModel.size(); i++)
	{
		m_vEffectModel[i].destory();
	}
	m_vEffectModel.clear();
}

void * CFaceEffect3DModel::Clone()
{
	CFaceEffect3DModel* result = new CFaceEffect3DModel();
	*result = *this;
	return result;
}

bool CFaceEffect3DModel::ReadConfig(XMLNode & childNode, HZIP hZip, char * pFilePath)
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

			const char* szGradient = nodeTime.getAttribute("Gradient");
			if (szGradient!=NULL)
			{
				sscanf(szGradient, "%f,%f,%f", m_GradientTime, m_GradientTime+1, m_GradientTime+2);
			}

		}

		XMLNode nodeMatCap = childNode.getChildNode("Matcap", 0);
		if (!nodeMatCap.isEmpty())
		{
			const char* szMatcap = nodeMatCap.getAttribute("image");
			const char* szNormal = nodeMatCap.getAttribute("normal");
			if (szMatcap != NULL)
			{
				m_MatCap = DXUtils::CreateTexFromZIP(hZip, szMatcap, false);
			}
			if (szNormal != NULL)
			{
				m_MatCapNormal = DXUtils::CreateTexFromZIP(hZip, szNormal, false);
			}
			m_hasMatcap = true;

		}

		XMLNode nodeObject = childNode.getChildNode("3DObject", 0);
		if (!nodeObject.isEmpty())
		{
			const char *szXml = nodeObject.getAttribute("xmlFile");
			if (szXml != NULL)
			{
				return loadFromXML(hZip, pFilePath, szXml);
			}
		}
	}

	return false;
}

bool CFaceEffect3DModel::Prepare()
{

	m_HeaderModel.m_arrIndex = g_headerModelForCullIndces;
	m_HeaderModel.m_arrPos = (vec3 *)g_headerModelForCullVertices;
	m_HeaderModel.m_nFaces = g_headerModelForCullFaceCount;
	m_HeaderModel.m_nVertex = g_headerModelForCullVerticesCount;

// 	int index3D[] = {
// 	347  ,
// 	338	 ,
// 	931	 ,
// 	940	 ,
// 	243	 ,
// 	210	 ,
// 	805	 ,
// 	837	 ,
// 	82	 ,
// 	78	 ,
// 	647	 ,
// 	662	 ,
// 	1207 ,
// 	560	 ,
// 	1127
// 	};
// 	for (int i = 0; i < 15; i++)
// 	{
// 		printf("%f,%f,%f,\n", m_HeaderModel.m_arrPos[index3D[i]].x, m_HeaderModel.m_arrPos[index3D[i]].y, m_HeaderModel.m_arrPos[index3D[i]].z);
// 	}


	m_HeaderModel.updateGpuBuffer();



	//����״̬
	if (m_pBSEnable == NULL)
	{
		m_pBSEnable = ContextInst->fetchBlendState(true, false, true);
	}
	if (m_pBSWriteDisable == NULL)
	{
		m_pBSWriteDisable = ContextInst->fetchBlendState(false, false, false);
	}
	if (m_pBSDisable == NULL)
	{
		m_pBSDisable = ContextInst->fetchBlendState(false, false, true);
	}
	if (m_pBSDisableWriteDisable == NULL)
	{
		m_pBSDisableWriteDisable = ContextInst->fetchBlendState(false, false, false);
	}

	//����״̬
	if (m_pDepthStateEnable == NULL)
	{
		m_pDepthStateEnable = ContextInst->fetchDepthStencilState(true, true);
	}
	if (m_pDepthStateDisable == NULL)
	{
		m_pDepthStateDisable = ContextInst->fetchDepthStencilState(false, false);
	}
	if (m_pDepthStateEnableWriteDisable == NULL)
	{
		m_pDepthStateEnableWriteDisable = ContextInst->fetchDepthStencilState(true, false);
	}

	m_pSamplerLinear = DXUtils::SetSampler();
	CCVetexAttribute pAttribute[] =
	{
		{VERTEX_ATTRIB_POSITION, FLOAT_C3},
		{VERTEX_ATTRIB_TEX_COORD, FLOAT_C2},
	};

	if (m_hasMatcap)
	{
		string path = m_resourcePath + "/Shader/face3dEffectMatCap.fx";
		m_pShader = ShaderProgramManager::GetInstance()->GetOrCreateShaderByPathAndAttribs(path, pAttribute, 2);
	}
	else
	{
		string path = m_resourcePath + "/Shader/face3dEffect.fx";
		m_pShader = ShaderProgramManager::GetInstance()->GetOrCreateShaderByPathAndAttribs(path, pAttribute, 2);
	}

	CCVetexAttribute pAttribute2[] =
	{
		{VERTEX_ATTRIB_POSITION, FLOAT_C3},
	};

	string path1 = m_resourcePath + "/Shader/face3dForCullFace.fx";
	m_pShaderForHeaderCull = ShaderProgramManager::GetInstance()->GetOrCreateShaderByPathAndAttribs(path1, pAttribute2, 1);

	m_pConstantBuffer = DXUtils::CreateConstantBuffer(sizeof(EffectConstantBuffer));
	m_pConstantBufferForHeaderCull = DXUtils::CreateConstantBuffer(sizeof(HeaderCullConstantBuffer));


	return true;
}


void CFaceEffect3DModel::Render(BaseRenderParam & RenderParam)
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
	int width = RenderParam.GetWidth();
	int height = RenderParam.GetHeight();
	int nFaceCount = RenderParam.GetFaceCount();
	float alpha = 1.0;
	if (m_GradientTime[0]>0&&runTime1<m_GradientTime[0])
	{
		alpha = runTime1 / m_GradientTime[0];
	}
	if (m_GradientTime[2]>m_GradientTime[1]&&runTime1>m_GradientTime[1])
	{
		alpha = (m_GradientTime[2] - runTime1) / (m_GradientTime[2] - m_GradientTime[1]);
		if (alpha < 0)
		{
			alpha = 0;
		}
	}

	if (nFaceCount > 0)
	{
		for (int faceIndex = 0; faceIndex < nFaceCount; faceIndex++)
		{
			FacePosInfo *pFaceInfo = RenderParam.GetFaceInfo(faceIndex);
			
			Mat4 matRotateX;
			Mat4 matRotateY;
			Mat4 matRotateZ;
			Mat4 matTranslate;
			Mat4 matRotateXYZ;
			Mat4::createRotationX(-pFaceInfo->pitch / 180.0f*PI, &matRotateX);
			Mat4::createRotationY(-pFaceInfo->yaw / 180.0f*PI, &matRotateY);
			Mat4::createRotationZ(pFaceInfo->roll / 180.0f*PI, &matRotateZ);

			Mat4::multiply(matRotateX, matRotateY, &matRotateXYZ);
			Mat4::multiply(matRotateXYZ, matRotateZ, &matRotateXYZ);
			Mat4::createTranslation(Vec3(pFaceInfo->x, pFaceInfo->y, -pFaceInfo->z), &matTranslate);
			Mat4::multiply(matTranslate, matRotateXYZ, &matRotateXYZ);
			memcpy(pFaceInfo->arrWMatrix, (float *)(&matRotateXYZ), sizeof(matRotateXYZ));
		}

		cocos2d::Vector<cocos2d::MeshRenderCmd *> m_renderCMD;

		if (m_nStartTime < 0)
		{
			m_nLastTime = m_nLoopStartTime = m_nStartTime = ccCurrentTime();
		}

		__int64 currentTime = ccCurrentTime();
		long runTime = long(currentTime - m_nStartTime);
		long loopTime = long(currentTime - m_nLoopStartTime);
		long during = runTime;

		long frameTime = (long)(currentTime - m_nLastTime);

		m_nLastTime = currentTime;
		if (m_nAniLoopTime > 0)
		{
			if (m_nEffectDuring < 0)
			{
				m_nEffectDuring = m_nAniLoopTime;
			}

			during = during % m_nAniLoopTime;
		}
		auto pDoubleBuffer = RenderParam.GetDoubleBuffer();

		pDoubleBuffer->BindFBOA();
		//��Ⱦ��ͷ����Ϣ,���ں����޳�(��ֹ���ϣ���ֹд����ɫbuffer���������Ȳ��Ժ�����bufferд)
		DeviceContextPtr->ClearDepthStencilView(pDoubleBuffer->GetFBOA()->getDepthStencilView(), D3D11_CLEAR_DEPTH, 1.0f, 0);
		DeviceContextPtr->OMSetDepthStencilState(m_pDepthStateEnable, 0);
		float blendFactor[] = { 0.f,0.f,0.f,0.f };
		DeviceContextPtr->OMSetBlendState(m_pBSDisableWriteDisable, blendFactor, 0xffffffff);
		m_pShaderForHeaderCull->useShader();

		for (int faceIndex = 0; faceIndex < nFaceCount; ++faceIndex)
		{
			//���������任������Ϣ
			FacePosInfo *pFaceInfo = RenderParam.GetFaceInfo(faceIndex);
			//set roi viewport
			D3D11_VIEWPORT vp;
			vp.Width = (FLOAT)pFaceInfo->pFaceRect.width;
			vp.Height = (FLOAT)pFaceInfo->pFaceRect.height;
			vp.MinDepth = 0.0f;
			vp.MaxDepth = 1.0f;
			vp.TopLeftX = pFaceInfo->pFaceRect.x;
			vp.TopLeftY = pFaceInfo->pFaceRect.y;
			DeviceContextPtr->RSSetViewports(1, &vp);

			Mat4 matRotateXYZ = Mat4(pFaceInfo->arrWMatrix);
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


			HeaderCullConstantBuffer matWVP;
			matWVP.mWVP = matRotateXYZ;
			//���þ����任
			DeviceContextPtr->UpdateSubresource(m_pConstantBufferForHeaderCull, 0, NULL, &matWVP, 0, 0);
			DeviceContextPtr->VSSetConstantBuffers(0, 1, &m_pConstantBufferForHeaderCull);

			//���ö�������
			unsigned int nStride = sizeof(vec3);
			unsigned int nOffset = 0;
			DeviceContextPtr->IASetVertexBuffers(0, 1, &m_HeaderModel.m_headerVerticeBuffer, &nStride, &nOffset);
			DeviceContextPtr->IASetIndexBuffer(m_HeaderModel.m_headerIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
			DeviceContextPtr->DrawIndexed(m_HeaderModel.m_nFaces * 3, 0, 0);
		}

		//���Ʋ�͸��������
		DeviceContextPtr->OMSetBlendState(m_pBSDisable, blendFactor, 0xffffffff);
		m_pShader->useShader();
		for (int faceIndex = 0; faceIndex < nFaceCount; ++faceIndex)
		{
			//���������任������Ϣ
			FacePosInfo *pFaceInfo = RenderParam.GetFaceInfo(faceIndex);
			//set roi viewport
			D3D11_VIEWPORT vp;
			vp.Width = (FLOAT)pFaceInfo->pFaceRect.width;
			vp.Height = (FLOAT)pFaceInfo->pFaceRect.height;
			vp.MinDepth = 0.0f;
			vp.MaxDepth = 1.0f;
			vp.TopLeftX = pFaceInfo->pFaceRect.x;
			vp.TopLeftY = pFaceInfo->pFaceRect.y;
			DeviceContextPtr->RSSetViewports(1, &vp);

			Mat4 matRotateXYZ = Mat4(pFaceInfo->arrWMatrix);

			Mat4 matNormal = matRotateXYZ;
			matNormal.inverse();
			matNormal.transpose();
			matNormal.m[12] = matNormal.m[13] = matNormal.m[14] = 0.0;
			matNormal.m[3] = matNormal.m[7] = matNormal.m[11] = 0.0;
			matNormal.m[15] = 1.0;
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
			EffectConstantBuffer matWVP;
			matWVP.mWVP = matRotateXYZ;
			matWVP.mNormal = matNormal;

			for (int i = 0; i < m_vEffectModel.size(); ++i)
			{
				EffectModel &model = m_vEffectModel[i];
				if (!model.m_bTransparent)
				{
					matWVP.mColor = XMVectorSet(model.m_fMixColor.x, model.m_fMixColor.y, model.m_fMixColor.z, model.m_fMixColor.w);

					//���þ����任
					DeviceContextPtr->UpdateSubresource(m_pConstantBuffer, 0, NULL, &matWVP, 0, 0);
					DeviceContextPtr->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
					DeviceContextPtr->PSSetConstantBuffers(0, 1, &m_pConstantBuffer);

					//���������Լ���������
					ID3D11ShaderResourceView *pMyShaderResourceView = model.m_drawable->GetSRV(runTime1);
					DeviceContextPtr->PSSetShaderResources(0, 1, &pMyShaderResourceView);
					if (m_hasMatcap)
					{
						pMyShaderResourceView = m_MatCap->getTexShaderView();
						DeviceContextPtr->PSSetShaderResources(1, 1, &pMyShaderResourceView);
						pMyShaderResourceView = m_MatCapNormal->getTexShaderView();
						DeviceContextPtr->PSSetShaderResources(2, 1, &pMyShaderResourceView);
					}

					DeviceContextPtr->PSSetSamplers(0, 1, &m_pSamplerLinear);



					//���ö�������
					unsigned int nStride = sizeof(EffectVertex);
					unsigned int nOffset = 0;
					DeviceContextPtr->IASetVertexBuffers(0, 1, &model.m_rectVerticeBuffer, &nStride, &nOffset);
					DeviceContextPtr->IASetIndexBuffer(model.m_rectIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

					DeviceContextPtr->DrawIndexed(model.m_nFaces * 3, 0, 0);
				}
			}
		}

		DeviceContextPtr->OMSetBlendState(m_pBSEnable, blendFactor, 0xffffffff);
		//DeviceContextPtr->OMSetDepthStencilState(m_pDepthStateEnableWriteDisable, 0);
		m_pShader->useShader();
		for (int faceIndex = 0; faceIndex < nFaceCount; ++faceIndex)
		{
			//���������任������Ϣ
			FacePosInfo *pFaceInfo = RenderParam.GetFaceInfo(faceIndex);
			//set roi viewport
			D3D11_VIEWPORT vp;
			vp.Width = (FLOAT)pFaceInfo->pFaceRect.width;
			vp.Height = (FLOAT)pFaceInfo->pFaceRect.height;
			vp.MinDepth = 0.0f;
			vp.MaxDepth = 1.0f;
			vp.TopLeftX = pFaceInfo->pFaceRect.x;
			vp.TopLeftY = pFaceInfo->pFaceRect.y;
			DeviceContextPtr->RSSetViewports(1, &vp);

			Mat4 matRotateXYZ = Mat4(pFaceInfo->arrWMatrix);
			
			Mat4 matProj;
			float fAngle = atan(0.5) * 2;
			if (pFaceInfo->pFaceRect.height > pFaceInfo->pFaceRect.width)
			{
				fAngle = atan(0.5*pFaceInfo->pFaceRect.height / pFaceInfo->pFaceRect.width) * 2;
			}
			XMMATRIX matProjDX = XMMatrixPerspectiveFovRH(fAngle, pFaceInfo->pFaceRect.width*1.f / pFaceInfo->pFaceRect.height, 1, 2000);
			matProjDX = XMMatrixTranspose(matProjDX);


			Mat4 matNormal = matRotateXYZ;
			matNormal.inverse();
			matNormal.transpose();
			matNormal.m[12] = matNormal.m[13] = matNormal.m[14] = 0.0;
			matNormal.m[3] = matNormal.m[7] = matNormal.m[11] = 0.0;
			matNormal.m[15] = 1.0;

			memcpy(&matProj, &matProjDX, sizeof(XMMATRIX));
			Mat4::multiply(matProj, matRotateXYZ, &matRotateXYZ);
			EffectConstantBuffer matWVP;
			matWVP.mWVP = matRotateXYZ;
			matWVP.mNormal = matNormal;
			for (int i = 0; i < m_vEffectModel.size(); ++i)
			{
				EffectModel &model = m_vEffectModel[i];
				if (model.m_bTransparent)
				{
					matWVP.mColor = XMVectorSet(model.m_fMixColor.x, model.m_fMixColor.y, model.m_fMixColor.z, model.m_fMixColor.w*alpha);

					//���þ����任
					DeviceContextPtr->UpdateSubresource(m_pConstantBuffer, 0, NULL, &matWVP, 0, 0);
					DeviceContextPtr->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
					DeviceContextPtr->PSSetConstantBuffers(0, 1, &m_pConstantBuffer);

					//���������Լ���������
					ID3D11ShaderResourceView *pMyShaderResourceView = model.m_drawable->GetSRV(runTime1);
					DeviceContextPtr->PSSetShaderResources(0, 1, &pMyShaderResourceView);
					if (m_hasMatcap)
					{
						pMyShaderResourceView = m_MatCap->getTexShaderView();
						DeviceContextPtr->PSSetShaderResources(1, 1, &pMyShaderResourceView);
						pMyShaderResourceView = m_MatCapNormal->getTexShaderView();
						DeviceContextPtr->PSSetShaderResources(2, 1, &pMyShaderResourceView);
					}
					DeviceContextPtr->PSSetSamplers(0, 1, &m_pSamplerLinear);

					//���ö�������
					unsigned int nStride = sizeof(EffectVertex);
					unsigned int nOffset = 0;
					DeviceContextPtr->IASetVertexBuffers(0, 1, &model.m_rectVerticeBuffer, &nStride, &nOffset);
					DeviceContextPtr->IASetIndexBuffer(model.m_rectIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

					ContextInst->setCullMode(D3D11_CULL_FRONT);
					DeviceContextPtr->DrawIndexed(model.m_nFaces * 3, 0, 0);

					ContextInst->setCullMode(D3D11_CULL_BACK);
					DeviceContextPtr->DrawIndexed(model.m_nFaces * 3, 0, 0);
				}
			}
		}
		DeviceContextPtr->OMSetDepthStencilState(m_pDepthStateDisable, 0);
		ContextInst->setCullMode(D3D11_CULL_NONE);
	}



}

bool CFaceEffect3DModel::loadFromXML(HZIP hZip, const char * szFilePath, const char * szFileXml)
{
	int index;
	ZIPENTRY ze;
	if (ZR_OK == FindZipItem(hZip, szFileXml, true, &index, &ze))
	{
		char *pDataBuffer = new char[ze.unc_size];
		ZRESULT res = UnzipItem(hZip, index, pDataBuffer, ze.unc_size);
		if (res != ZR_OK)
		{
			delete[]pDataBuffer;
			CloseZip(hZip);
			return false;
		}
		//����xml
		XMLResults xResults;
		XMLNode nodeModels = XMLNode::parseBuffer(pDataBuffer, ze.unc_size, "models", &xResults);
		delete[]pDataBuffer;
		typedef map<string, Drawable *> DrawableMap_;
		DrawableMap_ m_mapImage;
		int i = -1;
		XMLNode nodeDrawable = nodeModels.getChildNode("drawable", ++i);
		char szFullFile[256];
		while (!nodeDrawable.isEmpty())
		{
			const char *szDrawableName =    nodeDrawable.getAttribute("name");
			const char *szImagePath = nodeDrawable.getAttribute("image");

			sprintf(szFullFile, "%s/%s", szFilePath, szImagePath);

			const char *szGenMipmap = nodeDrawable.getAttribute("genMipmaps");
			bool bGenMipmap = false;
			if (szGenMipmap != NULL && (strcmp(szGenMipmap, "yes") || strcmp(szGenMipmap, "YES")))
			{
				bGenMipmap = true;
			}

			std::shared_ptr< CC3DTextureRHI> TexRHI = GetDynamicRHI()->FetchTexture(szFullFile, bGenMipmap);
			if (TexRHI == nullptr)
			{
				TexRHI = GetDynamicRHI()->CreateTextureFromZip(hZip, szImagePath, bGenMipmap);
				GetDynamicRHI()->RecoredTexture(szFullFile, TexRHI);
			}

			m_mapImage.insert(std::make_pair(szDrawableName, new BitmapDrawable(TexRHI)));


			nodeDrawable = nodeModels.getChildNode("drawable", ++i);
		}

		//���붯��drawable
		i = -1;
		nodeDrawable = nodeModels.getChildNode("anidrawable", ++i);
		while (!nodeDrawable.isEmpty())
		{
			const char *szDrawableName = nodeDrawable.getAttribute("name");
			AnimationDrawable *drawable = NULL;

			const char *szTarget = nodeDrawable.getAttribute("ref");
			if (szTarget != NULL)
			{
				map<std::string, Drawable *>::iterator it = m_mapImage.find(szTarget);
				if (it != m_mapImage.end())
				{
					AnimationDrawable *targetDrawable = (AnimationDrawable *)(it->second);
					drawable = (AnimationDrawable *)targetDrawable->Clone();
				}
			}
			if (drawable == NULL)
			{
				drawable = new AnimationDrawable();
			}

			const char *szOffset = nodeDrawable.getAttribute("offset");
			long offset = atol(szOffset);
			drawable->setOffset(offset);

			const char *szLoopMode = nodeDrawable.getAttribute("loopMode");
			if (szLoopMode != NULL && strcmp(szLoopMode, "oneShot") == 0)
			{
				drawable->setLoopMode(ELM_ONESHOT);
			}
			else
			{
				drawable->setLoopMode(ELM_REPEAT);
			}

			const char *szGenMipmap = nodeDrawable.getAttribute("genMipmaps");
			bool bGenMipmap = false;
			if (szGenMipmap != NULL && (strcmp(szGenMipmap, "yes") || strcmp(szGenMipmap, "YES")))
			{
				bGenMipmap = true;
			}
			//����������Ϣ
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
				if (nCount > 0 && nDuring > 0)
				{
					char szFullFile[256];
					for (; iStart <= nCount; iStart += nStep)
					{
						sprintf(szImagePath, szItems, iStart);

						sprintf(szFullFile, "%s/%s", szFilePath, szImagePath);

						std::shared_ptr< CC3DTextureRHI> TexRHI = GetDynamicRHI()->FetchTexture(szFullFile, bGenMipmap);
						if (TexRHI == nullptr)
						{
							TexRHI = GetDynamicRHI()->CreateTextureFromZip(hZip, szImagePath, bGenMipmap);
							GetDynamicRHI()->RecoredTexture(szFullFile, TexRHI);
						}

						long during = nDuring;

						drawable->appandTex(during, TexRHI);

					}
				}
			}

			m_mapImage.insert(std::make_pair(szDrawableName, drawable));

			nodeDrawable = nodeModels.getChildNode("anidrawable", ++i);
		}
		long aniLoopTime = -1;
		i = -1;
		XMLNode nodeModel = nodeModels.getChildNode("model", ++i);
		while (!nodeModel.isEmpty())
		{
			EffectModel model;
			//const char *szModelName = nodeModel.getAttribute("name");
			const char *szFaceCount = nodeModel.getAttribute("faceCount");
			const char *szVertexCount = nodeModel.getAttribute("vertexCount");
			//const char *szVertexDecl = nodeModel.getAttribute("vertexDecl");
			const char *szDrawable = nodeModel.getAttribute("drawable");
			const char *szColor = nodeModel.getAttribute("color");
			const char *szTransparent = nodeModel.getAttribute("transparent");
			if (szColor != NULL)
			{
				vec4 vColor(1, 1, 1, 1);
				sscanf(szColor, "%f,%f,%f,%f", &vColor.x, &vColor.y, &vColor.z, &vColor.w);
				model.m_fMixColor = vColor;
			}

			if (szTransparent != NULL)
			{
				model.m_bTransparent = false;
				if (strcmp(szTransparent, "YES") == 0 || strcmp(szTransparent, "yes") == 0)
				{
					model.m_bTransparent = true;
				}
			}

			if (szDrawable != NULL && strlen(szDrawable) > 0)
			{
				map<std::string, Drawable *>::iterator it = m_mapImage.find(szDrawable);
				if (it != m_mapImage.end())
				{
					model.m_drawable = it->second->Clone();
					if (model.m_drawable->getDuring() > aniLoopTime)
					{
						aniLoopTime = model.m_drawable->getDuring();
					}
				}
				else
				{
					model.m_drawable = NULL;
				}
			}

			const char *szModelRef = nodeModel.getAttribute("modeRef");
			if (szModelRef != NULL)
			{
				if (ZR_OK == FindZipItem(hZip, szModelRef, true, &index, &ze))
				{
					char *pDataBuffer = new char[ze.unc_size];
					ZRESULT res = UnzipItem(hZip, index, pDataBuffer, ze.unc_size);
					if (res == ZR_OK)
					{
						unsigned int nCode;
						int version;
						int nFaceCount;
						int nVertexCount;

						char *pDataTemp = pDataBuffer;
						memcpy(&nCode, pDataTemp, sizeof(unsigned int)); pDataTemp += 4;
						memcpy(&version, pDataTemp, sizeof(int)); pDataTemp += 4;
						if (nCode == 0x1f1f1f0f && version == 1)
						{
							memcpy(&nVertexCount, pDataTemp, sizeof(int)); pDataTemp += 4;

							model.m_nVertex = nVertexCount;
							model.m_arrVertices = new EffectVertex[nVertexCount];
							float *fDataTemp = (float *)pDataTemp;
							for (int indexVertex = 0; indexVertex < nVertexCount; ++indexVertex)
							{
								model.m_arrVertices[indexVertex].vPos = vec3(fDataTemp[0], fDataTemp[1], fDataTemp[2]);
								fDataTemp += 3;
							}
							for (int indexVertex = 0; indexVertex < nVertexCount; ++indexVertex)
							{
								model.m_arrVertices[indexVertex].vUV = vec2(fDataTemp[0], fDataTemp[1]);
								vec2 &vUV = model.m_arrVertices[indexVertex].vUV;
								if (vUV.x<0.0f || vUV.x > 1.0f)
								{
									vUV.x -= floor(vUV.x);
								}
								if (vUV.y<0.0f || vUV.y > 1.0f)
								{
									vUV.y -= floor(vUV.y);
								}
								vUV.y = 1.0f - vUV.y;

								fDataTemp += 2;
							}
							pDataTemp += sizeof(EffectVertex)*nVertexCount;

							//
							memcpy(&nFaceCount, pDataTemp, sizeof(int)); pDataTemp += 4;
							model.m_nFaces = nFaceCount;
							model.m_arrIndex = new unsigned short[nFaceCount * 3];
							memcpy(model.m_arrIndex, pDataTemp, sizeof(short)*nFaceCount * 3);
						}

						model.m_rectVerticeBuffer = DXUtils::CreateVertexBuffer((float*)model.m_arrVertices, model.m_nVertex, 5);
						model.m_rectIndexBuffer = DXUtils::CreateIndexBuffer(model.m_arrIndex, model.m_nFaces);
						m_vEffectModel.push_back(model);
					}
					delete[]pDataBuffer;
				}
			}
			else
			{
				int nFaceCount = atoi(szFaceCount);
				int nVertexCount = atoi(szVertexCount);
				//int nVertexDecl = atoi(szVertexDecl);

				if (nFaceCount > 0 && nVertexCount > 0)
				{
					model.m_nFaces = nFaceCount;
					model.m_nVertex = nVertexCount;
					model.m_arrVertices = new EffectVertex[nVertexCount];
					model.m_arrIndex = new unsigned short[nFaceCount * 3];

					int j = -1;
					XMLNode nodeVertex = nodeModel.getChildNode("vertex", ++j);
					while (!nodeVertex.isEmpty())
					{
						const char *szPos = nodeVertex.getAttribute("pos");
						const char *szUV = nodeVertex.getAttribute("uv");

						vec2 vUV;
						vec3 vPos;
						sscanf(szPos, "%f,%f,%f", &vPos.x, &vPos.y, &vPos.z);
						sscanf(szUV, "%f,%f", &vUV.x, &vUV.y);
						if (vUV.x<0.0f || vUV.x > 1.0f)
						{
							vUV.x -= floor(vUV.x);
						}
						if (vUV.y<0.0f || vUV.y > 1.0f)
						{
							vUV.y -= floor(vUV.y);
						}

						model.m_arrVertices[j].vPos = vPos;
						model.m_arrVertices[j].vPos.z = vPos.z;
						model.m_arrVertices[j].vUV = vUV;
						model.m_arrVertices[j].vUV.y = 1.0f - vUV.y;
						nodeVertex = nodeModel.getChildNode("vertex", ++j);
					}

					j = -1;
					XMLNode nodeIndex = nodeModel.getChildNode("face", ++j);
					while (!nodeIndex.isEmpty())
					{
						const char *szIndex = nodeIndex.getAttribute("index");
						int index[3];
						sscanf(szIndex, "%d,%d,%d", index, index + 1, index + 2);

						model.m_arrIndex[j * 3] = (short)index[0];
						model.m_arrIndex[j * 3 + 1] = (short)index[1];
						model.m_arrIndex[j * 3 + 2] = (short)index[2];
						nodeIndex = nodeModel.getChildNode("face", ++j);
					}
					model.m_rectVerticeBuffer = DXUtils::CreateVertexBuffer((float*)model.m_arrVertices, model.m_nVertex, 5);
					model.m_rectIndexBuffer = DXUtils::CreateIndexBuffer(model.m_arrIndex, model.m_nFaces);
					m_vEffectModel.push_back(model);
				}
			}

			nodeModel = nodeModels.getChildNode("model", ++i);
		}

		//std::sort(m_vEffectModel.begin(), m_vEffectModel.end(), EffectModel::ModeComp());

		return true;

	}
	return false;
}
