#include "CFaceEffect3DModelGL.h"
#include "Toolbox/HeaderModelForCull.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "Toolbox/GL/CCProgram.h"

CFaceEffect3DModelGL::CFaceEffect3DModelGL()
{
	m_nAniLoopTime = -1;

	m_pShaderForHeaderCull = NULL;

}


CFaceEffect3DModelGL::~CFaceEffect3DModelGL()
{
	Release();
}

void CFaceEffect3DModelGL::Release()
{

	for (int i = 0; i < m_vEffectModel.size(); i++) {
		SAFE_DELETE(m_vEffectModel[i]);
	}
	auto it = m_mapImage.begin();
	{
		while (it != m_mapImage.end())
		{
			if (it->second != NULL)
			{
				delete it->second;
			}
			++it;
		}
	}
	m_pShaderForHeaderCull.reset();
	m_pShader.reset();
}

void * CFaceEffect3DModelGL::Clone()
{
	CFaceEffect3DModelGL* result = new CFaceEffect3DModelGL();
	*result = *this;
	return result;
}

bool CFaceEffect3DModelGL::ReadConfig(XMLNode & childNode, HZIP hZip, char * pFilePath)
{
	if (!childNode.isEmpty())
	{
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

bool CFaceEffect3DModelGL::Prepare()
{
	m_pShader = std::make_shared<class CCProgram>();
	std::string  vspath = m_resourcePath + "/Shader/StickerEffect.vs";
	std::string  fspath = m_resourcePath + "/Shader/StickerEffect.fs";
	m_pShader->CreatePorgramForFile(vspath.c_str(), fspath.c_str());

	m_pShaderForHeaderCull = std::make_shared<class CCProgram>();
	vspath = m_resourcePath + "/Shader/HeadCullFace.vs";
	fspath = m_resourcePath + "/Shader/HeadCullFace.fs";
	m_pShaderForHeaderCull->CreatePorgramForFile(vspath.c_str(), fspath.c_str());

	return true;
}


void CFaceEffect3DModelGL::Render(BaseRenderParam & RenderParam)
{
	if (RenderParam.GetFaceCount() == 0)
	{
		return;
	}

	int width = RenderParam.GetWidth();
	int height = RenderParam.GetHeight();
	int nFaceCount = RenderParam.GetFaceCount();
	if (nFaceCount > 0)
	{
		Vector4 faceOriRect = Vector4(0.0, 0.0, 1.0, 1.0);
		FacePosInfo *pFaceInfo = RenderParam.GetFaceInfo(0);
		
		if (pFaceInfo->pFaceRect.width > 0 && pFaceInfo->pFaceRect.height > 0)
		{
			faceOriRect.x = pFaceInfo->pFaceRect.x * 1.f / width;
			faceOriRect.y = pFaceInfo->pFaceRect.y * 1.f / height;
			faceOriRect.z = pFaceInfo->pFaceRect.width * 1.f / width;
			faceOriRect.w = pFaceInfo->pFaceRect.height * 1.f / height;
			width = pFaceInfo->pFaceRect.width;
			height = pFaceInfo->pFaceRect.height;
		}

		//if (pFaceInfo->pFaceRect != NULL)
		//{
		//	faceOriRect.x = pFaceInfo->pFaceRect->x * 1.f / width;
		//	faceOriRect.y = pFaceInfo->pFaceRect->y * 1.f / height;
		//	faceOriRect.z = pFaceInfo->pFaceRect->width * 1.f / width;
		//	faceOriRect.w = pFaceInfo->pFaceRect->height * 1.f / height;
		//	width = pFaceInfo->pFaceRect->width;
		//	height = pFaceInfo->pFaceRect->height;
		//}

		
		auto pDoubleBuffer = RenderParam.GetDoubleBuffer();
		pDoubleBuffer->BindFBOA();

		//glViewport(0, 0, RenderParam.GetWidth(), RenderParam.GetHeight());
		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);
		glDepthFunc(GL_LEQUAL);
		glClear(GL_DEPTH_BUFFER_BIT);
		glColorMask(0, 0, 0, 0);
		m_pShaderForHeaderCull->Use();

		for (int faceIndex = 0; faceIndex < nFaceCount; ++faceIndex)
		{
			//���������任������Ϣ
			FacePosInfo* pFaceInfo = RenderParam.GetFaceInfo(faceIndex);
			
			glViewport(pFaceInfo->pCameraRect.x, pFaceInfo->pCameraRect.y, pFaceInfo->pCameraRect.width, pFaceInfo->pCameraRect.height);
			float fAngle = atan(0.5) * 2;
			if (0.5*pFaceInfo->pCameraRect.height > pFaceInfo->pCameraRect.width)
			{
				fAngle = atan(0.5*pFaceInfo->pCameraRect.height / pFaceInfo->pCameraRect.width) * 2;
			}
			glm::mat4 matView = glm::lookAt(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, -1.f), glm::vec3(0.f, 1.f, 0.f));
			glm::mat4 matProjDX = glm::perspective(fAngle, pFaceInfo->pCameraRect.width*1.f / pFaceInfo->pCameraRect.height, 10.0f, 2000.f);
			matProjDX = matProjDX * matView;


			//Mat4 matRotateXYZ = Mat4(pFaceInfo->arrWMatrix);
			glm::mat4 matRotateXYZ = glm::mat4(1.0f);
			memcpy((float *)(&matRotateXYZ[0][0]), pFaceInfo->arrWMatrix, sizeof(float) * 16);

			m_pShaderForHeaderCull->SetUniform4f("faceoriRect", faceOriRect.x, faceOriRect.y, faceOriRect.z, faceOriRect.w);
			glm::mat4 matWVP = matProjDX * matRotateXYZ;

			m_pShaderForHeaderCull->SetVertexAttribPointer("a_position", 3, GL_FLOAT, false, 0, g_headerModelForCullVertices);
			m_pShaderForHeaderCull->SetUniformMatrix4fv("u_matWVP", &matWVP[0][0], false, 1);
			glDrawElements(GL_TRIANGLES, g_headerModelForCullFaceCount * 3, GL_UNSIGNED_SHORT, g_headerModelForCullIndces);

		}

		//���Ʋ�͸��������
		glColorMask(1, 1, 1, 0);
		m_pShader->Use();
		for (int faceIndex = 0; faceIndex < nFaceCount; ++faceIndex)
		{
			//���������任������Ϣ
			FacePosInfo* pFaceInfo = RenderParam.GetFaceInfo(faceIndex);
			glViewport(pFaceInfo->pCameraRect.x, pFaceInfo->pCameraRect.y, pFaceInfo->pCameraRect.width, pFaceInfo->pCameraRect.height);
			
			float fAngle = atan(0.5) * 2;
			if (0.5*pFaceInfo->pCameraRect.height > pFaceInfo->pCameraRect.width)
			{
				fAngle = atan(0.5*pFaceInfo->pCameraRect.height / pFaceInfo->pCameraRect.width) * 2;
			}
			glm::mat4 matView = glm::lookAt(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, -1.f), glm::vec3(0.f, 1.f, 0.f));
			glm::mat4 matProjDX = glm::perspective(fAngle, pFaceInfo->pCameraRect.width*1.f / pFaceInfo->pCameraRect.height, 10.0f, 2000.f);
			matProjDX = matProjDX * matView;

			//Mat4 matRotateXYZ = Mat4(pFaceInfo->arrWMatrix);
			glm::mat4 matRotateXYZ = glm::mat4(1.0f);
			memcpy((float *)(&matRotateXYZ[0][0]), pFaceInfo->arrWMatrix, sizeof(float) * 16);

			glm::mat4 matWVP = matProjDX * matRotateXYZ;

			for (int i = 0; i < m_vEffectModel.size(); ++i)
			{
				CCEffectModel *model = m_vEffectModel[i];
				if (!model->m_bTransparent)
				{
					m_pShader->SetUniformMatrix4fv("u_matWVP", &matWVP[0][0], false, 1);
					std::shared_ptr<MaterialTexRHI> pTex = NULL;
					if (model->m_drawable != NULL)
					{
						pTex = model->m_drawable->GetTex(RenderParam.runtime);
					}
					if (pTex == NULL)
					{
						continue;
					}
					pTex->Bind(0);

					m_pShader->SetUniform4f("faceoriRect", faceOriRect.x, faceOriRect.y, faceOriRect.z, faceOriRect.w);
					m_pShader->SetUniformMatrix4fv("u_matWVP", &matWVP[0][0], false, 1);
					m_pShader->SetVertexAttribPointer("a_position", 3, GL_FLOAT, false, 0, model->m_arrPos);
					m_pShader->SetVertexAttribPointer("a_inputTextureCoordinate", 2, GL_FLOAT, false, 0, model->m_arrUV);
					m_pShader->SetUniform4f("u_color", model->m_fMixColor.x, model->m_fMixColor.y, model->m_fMixColor.z, model->m_fMixColor.w);
					glDrawElements(GL_TRIANGLES, model->m_nFaces * 3, GL_UNSIGNED_SHORT, model->m_arrIndex);

				}
			}
		}
		//绘制透明模型效果
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDepthMask(GL_FALSE);
		glColorMask(1, 1, 1, 1);
		m_pShader->Use();
		for (int faceIndex = 0; faceIndex < nFaceCount; ++faceIndex)
		{
			FacePosInfo* pFaceInfo = RenderParam.GetFaceInfo(faceIndex);
			glViewport(pFaceInfo->pCameraRect.x, pFaceInfo->pCameraRect.y, pFaceInfo->pCameraRect.width, pFaceInfo->pCameraRect.height);
			
			float fAngle = atan(0.5) * 2;
			if (0.5*pFaceInfo->pCameraRect.height > pFaceInfo->pCameraRect.width)
			{
				fAngle = atan(0.5*pFaceInfo->pCameraRect.height / pFaceInfo->pCameraRect.width) * 2;
			}
			glm::mat4 matView = glm::lookAt(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, -1.f), glm::vec3(0.f, 1.f, 0.f));
			glm::mat4 matProjDX = glm::perspective(fAngle, pFaceInfo->pCameraRect.width*1.f / pFaceInfo->pCameraRect.height, 10.0f, 2000.f);
			matProjDX = matProjDX * matView;

			glm::mat4 matRotateXYZ = glm::mat4(1.0f);
			memcpy((float *)(&matRotateXYZ[0][0]), pFaceInfo->arrWMatrix, sizeof(float) * 16);

			glm::mat4 matWVP = matProjDX * matRotateXYZ;
			for (int i = 0; i < m_vEffectModel.size(); ++i)
			{
				CCEffectModel *model = m_vEffectModel[i];
				if (model->m_bTransparent)
				{
					m_pShader->SetUniformMatrix4fv("u_matWVP", &matWVP[0][0], false, 1);
					std::shared_ptr<MaterialTexRHI> pTex;
					if (model->m_drawable != NULL)
					{
						pTex = model->m_drawable->GetTex(RenderParam.runtime);
					}
					if (pTex == NULL)
					{
						continue;
					}
					pTex->Bind(0);
					//
					m_pShader->SetUniform4f("faceoriRect", faceOriRect.x, faceOriRect.y, faceOriRect.z, faceOriRect.w);
					m_pShader->SetUniformMatrix4fv("u_matWVP", &matWVP[0][0], false, 1);
					m_pShader->SetVertexAttribPointer("a_position", 3, GL_FLOAT, false, 0, model->m_arrPos);
					m_pShader->SetVertexAttribPointer("a_inputTextureCoordinate", 2, GL_FLOAT, false, 0, model->m_arrUV);
					m_pShader->SetUniform4f("u_color", model->m_fMixColor.x, model->m_fMixColor.y, model->m_fMixColor.z, model->m_fMixColor.w);
					glDrawElements(GL_TRIANGLES, model->m_nFaces * 3, GL_UNSIGNED_SHORT, model->m_arrIndex);

				}
			}
		}
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);
		glDepthMask(GL_TRUE);
	}
}

bool CFaceEffect3DModelGL::loadFromXML(HZIP hZip, const char * szFilePath, const char * szFileXml)
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
		//typedef map<string, CCDrawable *> DrawableMap_;
		//DrawableMap_ m_mapImage;
		int i = -1;
		XMLNode nodeDrawable = nodeModels.getChildNode("drawable", ++i);
		char szFullFile[256];
		while (!nodeDrawable.isEmpty())
		{
			const char *szDrawableName = nodeDrawable.getAttribute("name");
			const char *szImagePath = nodeDrawable.getAttribute("image");

			sprintf(szFullFile, "%s/%s", szFilePath, szImagePath);

			const char *szGenMipmap = nodeDrawable.getAttribute("genMipmaps");
			bool bGenMipmap = false;
			if (szGenMipmap != NULL && (strcmp(szGenMipmap, "yes") || strcmp(szGenMipmap, "YES")))
			{
				bGenMipmap = true;
			}

			//MaterialTex *pTex = NULL;
			//if (pTex == NULL)
			//{
			//	pTex = new MaterialTex();
			//	pTex->CreateTexFromZIP(hZip, szImagePath);
			//}
			std::shared_ptr<MaterialTexRHI> pTex = GetDynamicRHI()->CreateAsynTextureZIP(hZip, szImagePath, bGenMipmap);
			m_mapImage.insert(std::make_pair(szDrawableName, new BitmapDrawable(pTex)));

			nodeDrawable = nodeModels.getChildNode("drawable", ++i);
		}


		i = -1;
		nodeDrawable = nodeModels.getChildNode("anidrawable", ++i);
		while (!nodeDrawable.isEmpty())
		{
			const char *szDrawableName = nodeDrawable.getAttribute("name");
			AnimationDrawable *drawable = NULL;

			//			const char *szTarget = nodeDrawable.getAttribute("ref");
			//			if (szTarget != NULL)
			//			{
			//                map<string, CCDrawable *>::iterator it = m_mapImage.find(szTarget);
			//				if (it != m_mapImage.end())
			//				{
			//                    CCAnimationDrawable *targetDrawable = (CCAnimationDrawable *)(it->second);
			//                    drawable = (CCAnimationDrawable *)targetDrawable->Clone();
			//				}
			//			}
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
						//MaterialTex *pTex = NULL;
						//if (pTex == NULL)
						//{
						//	pTex = new MaterialTex();
						//	pTex->CreateTexFromZIP(hZip, szImagePath);
						//}
						std::shared_ptr<MaterialTexRHI> pTex = GetDynamicRHI()->CreateAsynTextureZIP(hZip, szImagePath, bGenMipmap);

						long during = nDuring;

						drawable->appandTex(during, pTex);
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
			CCEffectModel *model = new CCEffectModel();
			//const char *szModelName = nodeModel.getAttribute("name");
			const char *szFaceCount = nodeModel.getAttribute("faceCount");
			const char *szVertexCount = nodeModel.getAttribute("vertexCount");
			//const char *szVertexDecl = nodeModel.getAttribute("vertexDecl");
			const char *szDrawable = nodeModel.getAttribute("drawable");
			const char *szColor = nodeModel.getAttribute("color");
			const char *szTransparent = nodeModel.getAttribute("transparent");
			if (szColor != NULL)
			{
				Vector4 vColor(1, 1, 1, 1);
				sscanf(szColor, "%f,%f,%f,%f", &vColor.x, &vColor.y, &vColor.z, &vColor.w);
				model->m_fMixColor = vColor;
			}

			if (szTransparent != NULL)
			{
				model->m_bTransparent = false;
				if (strcmp(szTransparent, "YES") == 0 || strcmp(szTransparent, "yes") == 0)
				{
					model->m_bTransparent = true;
				}
			}

			if (szDrawable != NULL && strlen(szDrawable) > 0)
			{
				auto  it = m_mapImage.find(szDrawable);
				if (it != m_mapImage.end())
				{
					model->m_drawable = it->second;//->Clone();
					if (model->m_drawable->getDuring() > aniLoopTime)
					{
						aniLoopTime = model->m_drawable->getDuring();
					}
				}
				else
				{
					model->m_drawable = NULL;
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
							model->m_nVertex = nVertexCount;

							model->m_arrPos = new Vector3[nVertexCount];
							model->m_arrUV = new Vector2[nVertexCount];

							//model->m_nVertex = nVertexCount;
							//model.m_arrVertices = new EffectVertex[nVertexCount];
							float *fDataTemp = (float *)pDataTemp;
							for (int indexVertex = 0; indexVertex < nVertexCount; ++indexVertex)
							{
								model->m_arrPos[indexVertex] = Vector3(fDataTemp[0], fDataTemp[1], fDataTemp[2]);
								fDataTemp += 3;
							}
							for (int indexVertex = 0; indexVertex < nVertexCount; ++indexVertex)
							{
								model->m_arrUV[indexVertex] = Vector2(fDataTemp[0], fDataTemp[1]);
								Vector2 &vUV = model->m_arrUV[indexVertex];
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
							pDataTemp += sizeof(float) * 5 * nVertexCount;

							//index
							memcpy(&nFaceCount, pDataTemp, sizeof(int)); pDataTemp += 4;
							model->m_nFaces = nFaceCount;
							model->m_arrIndex = new short[nFaceCount * 3];
							memcpy(model->m_arrIndex, pDataTemp, sizeof(short)*nFaceCount * 3);
						}
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
					model->m_nFaces = nFaceCount;
					model->m_nVertex = nVertexCount;
					//model.m_arrVertices = new EffectVertex[nVertexCount];
					model->m_arrPos = new Vector3[nVertexCount];
					model->m_arrUV = new Vector2[nVertexCount];
					model->m_arrIndex = new short[nFaceCount * 3];

					int j = -1;
					XMLNode nodeVertex = nodeModel.getChildNode("vertex", ++j);
					while (!nodeVertex.isEmpty())
					{
						const char *szPos = nodeVertex.getAttribute("pos");
						const char *szUV = nodeVertex.getAttribute("uv");

						Vector2 vUV;
						Vector3 vPos;
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

						model->m_arrPos[j] = vPos;
						model->m_arrUV[j] = vUV;

						nodeVertex = nodeModel.getChildNode("vertex", ++j);
					}

					j = -1;
					XMLNode nodeIndex = nodeModel.getChildNode("face", ++j);
					while (!nodeIndex.isEmpty())
					{
						const char *szIndex = nodeIndex.getAttribute("index");
						int index[3];
						sscanf(szIndex, "%d,%d,%d", index, index + 1, index + 2);

						model->m_arrIndex[j * 3] = (short)index[0];
						model->m_arrIndex[j * 3 + 1] = (short)index[1];
						model->m_arrIndex[j * 3 + 2] = (short)index[2];
						nodeIndex = nodeModel.getChildNode("face", ++j);
					}
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
