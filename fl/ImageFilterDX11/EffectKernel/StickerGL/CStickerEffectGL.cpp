#include "CStickerEffectGL.h"
#include <algorithm>
#include "Toolbox/zip/unzip.h"
#include "Toolbox/HeaderModelForCull.h"
#include "Toolbox/GL/CCProgram.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "Toolbox/Drawable.hpp"
#include "Toolbox/GL/GLResource.h"

CStickerEffectGL::CStickerEffectGL()
{
     m_nAniLoopTime = -1;
     m_pShaderHead = NULL;
     m_pShader2D = NULL;
}


CStickerEffectGL::~CStickerEffectGL()
{
	Release();
}

void * CStickerEffectGL::Clone()
{
    CStickerEffectGL* result = new CStickerEffectGL();
	*result = *this;
	return result;
}

bool CStickerEffectGL::ReadConfig(XMLNode & childNode, HZIP hZip , char *pFilePath)
{
    if (!childNode.isEmpty())
    {
        XMLNode &nodeModels = childNode;


        int i = -1;
        XMLNode nodeDrawable = nodeModels.getChildNode("drawable", ++i);
        while (!nodeDrawable.isEmpty())
        {
            const char *szDrawableName = nodeDrawable.getAttribute("name");
            const char *szImagePath = nodeDrawable.getAttribute("image");
            if(m_mapImage.find(szDrawableName)==m_mapImage.end())
            {
                //MaterialTex *pTex = new MaterialTex();
                //pTex->CreateTexFromZIP(hZip,szImagePath);
                std::shared_ptr<MaterialTexRHI> TexRHI = GetDynamicRHI()->CreateAsynTextureZIP(hZip, szImagePath);
                m_mapImage.insert(std::make_pair(szDrawableName,new BitmapDrawable (TexRHI)));
            }

            nodeDrawable = nodeModels.getChildNode("drawable", ++i);
        }

        i = -1;
       nodeDrawable = nodeModels.getChildNode("anidrawable", ++i);
       while (!nodeDrawable.isEmpty())
       {
           const char *szDrawableName = nodeDrawable.getAttribute("name");
           AnimationDrawable *drawable = NULL;

//           const char *szTarget = nodeDrawable.getAttribute("ref");
//           if (szTarget != NULL)
//           {
//               map<string, Drawable *>::iterator it = m_mapImage.find(szTarget);
//               if (it != m_mapImage.end())
//               {
//                   CCAnimationDrawable *targetDrawable = (CCAnimationDrawable *)(it->second);
//                   drawable = (CCAnimationDrawable *)targetDrawable->Clone();
//               }
//           }
           if(drawable == NULL)
           {
               drawable = new AnimationDrawable();
           }

           const char *szOffset = nodeDrawable.getAttribute("offset");
           long offset = atol(szOffset);
           drawable->setOffset(offset);

           const char *szLoopMode = nodeDrawable.getAttribute("loopMode");
           if(szLoopMode != NULL && strcmp(szLoopMode, "oneShot")==0)
           {
               drawable->setLoopMode(ELM_ONESHOT);
           }
           else
           {
               drawable->setLoopMode(ELM_REPEAT);
           }

           int j = -1;
           XMLNode nodeItem = nodeDrawable.getChildNode("item", ++j);
           while (!nodeItem.isEmpty())
           {
               const char *szImagePath = nodeItem.getAttribute("image");
              // MaterialTex *pTex = new MaterialTex();
              // pTex->CreateTexFromZIP(hZip,szImagePath);
               std::shared_ptr<MaterialTexRHI> TexRHI = GetDynamicRHI()->CreateAsynTextureZIP(hZip, szImagePath);
               const char *szDuring = nodeItem.getAttribute("duration");
               long during = atol(szDuring);
               drawable->appandTex(during, TexRHI);

               nodeItem = nodeDrawable.getChildNode("item", ++j);
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
           const char *szDrawable = nodeModel.getAttribute("drawable");
           const char *szColor = nodeModel.getAttribute("color");
           const char *szTransparent = nodeModel.getAttribute("transparent");
           if(szColor != NULL)
           {
               Vector4 vColor(1,1,1,1);
               sscanf(szColor, "%f,%f,%f,%f", &vColor.x, &vColor.y, &vColor.z, &vColor.w);
               model->m_fMixColor = vColor;
           }

           if(szTransparent != NULL)
           {
               model->m_bTransparent = false;
               if(strcmp(szTransparent, "YES") == 0)
               {
                   model->m_bTransparent = true;
               }
           }

           if (szDrawable != NULL && strlen(szDrawable) > 0)
           {
               auto it = m_mapImage.find(szDrawable);
               if (it != m_mapImage.end())
               {
                   model->m_drawable = it->second;//->Clone();
                   if(model->m_drawable->getDuring() > aniLoopTime)
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
           if(szModelRef != NULL)
           {
                ZIPENTRY ze;
                int index;
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
                       memcpy(&nCode, pDataTemp, sizeof(unsigned int)); pDataTemp+=4;
                       memcpy(&version, pDataTemp, sizeof(int)); pDataTemp += 4;
                       if(nCode == 0x1f1f1f0f && version == 1)
                       {
                           memcpy(&nVertexCount, pDataTemp, sizeof(int));pDataTemp+=4;

                           model->m_nVertex = nVertexCount;
                           model->m_arrPos = new Vector3[nVertexCount];
                           model->m_arrUV = new Vector2[nVertexCount];
                           memcpy(model->m_arrPos, pDataTemp, sizeof(Vector3)*nVertexCount); pDataTemp+=sizeof(Vector3)*nVertexCount;
                           memcpy(model->m_arrUV, pDataTemp, sizeof(Vector2)*nVertexCount); pDataTemp+=sizeof(Vector2)*nVertexCount;

                           //
                           memcpy(&nFaceCount, pDataTemp, sizeof(int)); pDataTemp+=4;
                           model->m_nFaces = nFaceCount;
                           model->m_arrIndex = new short[nFaceCount * 3];
                           memcpy(model->m_arrIndex, pDataTemp, sizeof(short)*nFaceCount*3);
                       }

                       m_vEffectModel.push_back(model);
                   }
                   delete []pDataBuffer;
               }
           }
           else
           {
               const char *szFaceCount = nodeModel.getAttribute("faceCount");
               const char *szVertexCount = nodeModel.getAttribute("vertexCount");
               int nFaceCount = atoi(szFaceCount);
               int nVertexCount = atoi(szVertexCount);
               //int nVertexDecl = atoi(szVertexDecl);

               if (nFaceCount > 0 && nVertexCount > 0)
               {
                   model->m_nFaces = nFaceCount;
                   model->m_nVertex = nVertexCount;
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
                       if(vUV.x > 1.0f || vUV.x < 0.0f)
                       {
                           vUV.x -= floor(vUV.x);
                       }
                       if(vUV.y > 1.0f || vUV.y < 0.0f)
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

       i = -1;
       XMLNode nodeRect = nodeModels.getChildNode("rect", ++i);
       while (!nodeRect.isEmpty())
       {
           /*<rect name="cao" drawable="cao" x="100" y="100" width="100" height="100" alignType="EAPT_LT">
            </rect>
            */
           CCEffect2DRect *rectEffect = new CCEffect2DRect();
           const char *szX = nodeRect.getAttribute("x");
           const char *szY = nodeRect.getAttribute("y");
           const char *szWidth = nodeRect.getAttribute("width");
           const char *szHeight = nodeRect.getAttribute("height");
           const char *szAlignType = nodeRect.getAttribute("alignType");

           int x = atoi(szX);
           int y = atoi(szY);
           int width = atoi(szWidth);
           int height = atoi(szHeight);
           int alignType = EAPT_CC;
           if(szAlignType != NULL)
           {
               std::string szArrAlignType[] = {"EAPT_LT", "EAPT_LB", "EAPT_RT", "EAPT_RB", "EAPT_CT", "EAPT_CB", "EAPT_LC", "EAPT_RC", "EAPT_CC"};
               for(int index=0; index<EAPT_MAX; ++index)
               {
                   if(strcmp(szAlignType, szArrAlignType[index].c_str()) == 0)
                   {
                       alignType = index;
                       break;
                   }
               }
           }

           rectEffect->setRect(x, y, width, height, alignType);

           const char *szDrawable = nodeRect.getAttribute("drawable");
           if (szDrawable != NULL && strlen(szDrawable) > 0)
           {
               auto it = m_mapImage.find(szDrawable);
               if (it != m_mapImage.end())
               {
                   rectEffect->m_drawable = it->second;//->Clone();
                   if(rectEffect->m_drawable->getDuring() > aniLoopTime)
                   {
                       aniLoopTime = rectEffect->m_drawable->getDuring();
                   }
               }
               else
               {
                   rectEffect->m_drawable = NULL;
               }
           }

           m_v2DEffectModel.push_back(rectEffect);
           nodeRect = nodeModels.getChildNode("rect", ++i);
       }

       m_nAniLoopTime = aniLoopTime;
       std::sort(m_vEffectModel.begin(), m_vEffectModel.end(), CCEffectModel::ModeComp());

        return true;
    }
    return false;
}



bool CStickerEffectGL::Prepare()
{
    m_pShader = std::make_shared<CCProgram>();
    std::string  vspath = m_resourcePath + "/Shader/StickerEffect.vs";
    std::string  fspath = m_resourcePath + "/Shader/StickerEffect.fs";
    m_pShader->CreatePorgramForFile(vspath.c_str(),fspath.c_str());

    m_pShaderHead = std::make_shared<CCProgram>();
    vspath = m_resourcePath + "/Shader/HeadCullFace.vs";
    fspath = m_resourcePath + "/Shader/HeadCullFace.fs";
    m_pShaderHead->CreatePorgramForFile(vspath.c_str(), fspath.c_str());


    m_pShader2D = std::make_shared<CCProgram>();
    vspath = m_resourcePath + "/Shader/faceEffectMakeUp.vs";
    fspath = m_resourcePath + "/Shader/faceEffectMakeUp.fs";
    m_pShader2D->CreatePorgramForFile(vspath.c_str(), fspath.c_str());

    return true;
}

void CStickerEffectGL::Render(BaseRenderParam &RenderParam)
{
    if(m_alpha<0.001f||(m_vEffectModel.size()<1&&m_v2DEffectModel.size()<1))
    {
        return;
    }

    auto pDoubleBuffer = RenderParam.GetDoubleBuffer();
    pDoubleBuffer->BindFBOA();
    
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glClear(GL_DEPTH_BUFFER_BIT);

    int width = RenderParam.GetWidth();
    int height = RenderParam.GetHeight();

	Vector4 faceOriRect = Vector4(0.0, 0.0, 1.0, 1.0);
	int nFaceCount = RenderParam.GetFaceCount();
	if (nFaceCount > 0)
	{
		
		FacePosInfo *pFaceInfo = RenderParam.GetFaceInfo(0);
		if (pFaceInfo->pFaceRect.width > 0 && pFaceInfo->pFaceRect.height > 0)
		{
			faceOriRect.x = pFaceInfo->pFaceRect.x *1.f/ width;
			faceOriRect.y = pFaceInfo->pFaceRect.y*1.f / height;
			faceOriRect.z = pFaceInfo->pFaceRect.width*1.f / width;
			faceOriRect.w = pFaceInfo->pFaceRect.height*1.f / height;
			width = pFaceInfo->pFaceRect.width;
			height = pFaceInfo->pFaceRect.height;
		}

	}

    glColorMask(0, 0, 0, 0);
  //  glColorMask(1, 1, 1, 1);

    m_pShaderHead->Use();
    for (int faceIndex = 0; faceIndex < nFaceCount; faceIndex++)
    {
		auto pFaceInfo = RenderParam.GetFaceInfo(faceIndex);

		float fAngle = atan(0.5) * 2;
		if (0.5*pFaceInfo->pCameraRect.height > pFaceInfo->pCameraRect.width)
		{
			fAngle = atan(0.5*pFaceInfo->pCameraRect.height / pFaceInfo->pCameraRect.width) * 2;
		}
		glm::mat4 matView = glm::lookAt(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, -1.f), glm::vec3(0.f, 1.f, 0.f));
		glm::mat4 matProjDX = glm::perspective(fAngle, pFaceInfo->pCameraRect.width*1.f / pFaceInfo->pCameraRect.height, 10.0f, 2000.f);

		glm::mat4 matScale3 = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f));

		matProjDX = matScale3 * matProjDX * matView;


		int faceL = pFaceInfo->pCameraRect.x;
		int faceT = pFaceInfo->pCameraRect.y;
		int faceW = pFaceInfo->pCameraRect.width;
		int faceH = pFaceInfo->pCameraRect.height;
		faceL -= 0.5*faceW;
		faceT -= 0.5*faceH;
		faceW *= 2;
		faceH *= 2;
		glViewport(faceL, faceT, faceW, faceH);


        auto info = RenderParam.GetFaceInfo(faceIndex);
        glm::mat4 matRotateXYZ;
        memcpy((float *)(&matRotateXYZ[0][0]), info->arrWMatrix, sizeof(float)*16);
	    m_pShaderHead->SetUniform4f("faceoriRect", faceOriRect.x, faceOriRect.y, faceOriRect.z, faceOriRect.w);

        glm::mat4 matWVP = matProjDX*matRotateXYZ;

        m_pShaderHead->SetVertexAttribPointer("a_position", 3, GL_FLOAT, false, 0, g_headerModelForCullVertices);
        m_pShaderHead->SetUniformMatrix4fv("u_matWVP",&matWVP[0][0],false,1);
        glDrawElements(GL_TRIANGLES, g_headerModelForCullFaceCount * 3, GL_UNSIGNED_SHORT, g_headerModelForCullIndces);


    }

    glColorMask(1, 1, 1, 0);

    m_pShader->Use();

    for (int faceIndex = 0; faceIndex < nFaceCount; faceIndex++)
    {
		auto pFaceInfo = RenderParam.GetFaceInfo(faceIndex);

		float fAngle = atan(0.5) * 2;
		if (0.5*pFaceInfo->pCameraRect.height > pFaceInfo->pCameraRect.width)
		{
			fAngle = atan(0.5*pFaceInfo->pCameraRect.height / pFaceInfo->pCameraRect.width) * 2;
		}
		glm::mat4 matView = glm::lookAt(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, -1.f), glm::vec3(0.f, 1.f, 0.f));
		glm::mat4 matProjDX = glm::perspective(fAngle, pFaceInfo->pCameraRect.width*1.f / pFaceInfo->pCameraRect.height, 10.0f, 2000.f);

		glm::mat4 matScale3 = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f));

		matProjDX = matScale3 * matProjDX * matView;


		int faceL = pFaceInfo->pCameraRect.x;
		int faceT = pFaceInfo->pCameraRect.y;
		int faceW = pFaceInfo->pCameraRect.width;
		int faceH = pFaceInfo->pCameraRect.height;
		faceL -= 0.5*faceW;
		faceT -= 0.5*faceH;
		faceW *= 2;
		faceH *= 2;
		glViewport(faceL, faceT, faceW, faceH);

        auto info = RenderParam.GetFaceInfo(faceIndex);
        glm::mat4 matWorld;
        memcpy((float *)(&matWorld[0][0]), info->arrWMatrix, sizeof(float)*16);

        glm::mat4  matWVP = matProjDX*matWorld;

        for (int i = 0; i<m_vEffectModel.size(); ++i)
        {
            CCEffectModel *model = m_vEffectModel[i];
            if(!model->m_bTransparent)
            {
                m_pShader->SetUniformMatrix4fv("u_matWVP",&matWVP[0][0],false,1);
                std::shared_ptr<MaterialTexRHI> pTex ;
                if (model->m_drawable != NULL)
                {
                    pTex =model->m_drawable->GetTex(RenderParam.runtime);

                }
                if(pTex == NULL || pTex->GetWidth() == 0)
                {
                    continue;
                }
                pTex->Bind(0);
                m_pShader->SetUniformMatrix4fv("u_matWVP",&matWVP[0][0],false,1);
                m_pShader->SetVertexAttribPointer("a_position", 3, GL_FLOAT, false, 0, model->m_arrPos);
                m_pShader->SetVertexAttribPointer("a_inputTextureCoordinate", 2, GL_FLOAT, false, 0, model->m_arrUV);
                m_pShader->SetUniform4f("u_color",model->m_fMixColor.x,model->m_fMixColor.y,model->m_fMixColor.z,model->m_fMixColor.w);
				m_pShader->SetUniform4f("faceoriRect", faceOriRect.x, faceOriRect.y, faceOriRect.z, faceOriRect.w);
                glDrawElements(GL_TRIANGLES, model->m_nFaces * 3, GL_UNSIGNED_SHORT, model->m_arrIndex);
            }

        }
    }
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);
 
    for (int faceIndex = 0; faceIndex < nFaceCount; faceIndex++)
    {
		auto pFaceInfo = RenderParam.GetFaceInfo(faceIndex);

		float fAngle = atan(0.5) * 2;
		if (0.5*pFaceInfo->pCameraRect.height > pFaceInfo->pCameraRect.width)
		{
			fAngle = atan(0.5*pFaceInfo->pCameraRect.height / pFaceInfo->pCameraRect.width) * 2;
		}
		glm::mat4 matView = glm::lookAt(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, -1.f), glm::vec3(0.f, 1.f, 0.f));
		glm::mat4 matProjDX = glm::perspective(fAngle, pFaceInfo->pCameraRect.width*1.f / pFaceInfo->pCameraRect.height, 10.0f, 2000.f);

		glm::mat4 matScale3 = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f));

		matProjDX = matScale3 * matProjDX * matView;


		int faceL = pFaceInfo->pCameraRect.x;
		int faceT = pFaceInfo->pCameraRect.y;
		int faceW = pFaceInfo->pCameraRect.width;
		int faceH = pFaceInfo->pCameraRect.height;
		faceL -= 0.5*faceW;
		faceT -= 0.5*faceH;
		faceW *= 2;
		faceH *= 2;
		glViewport(faceL, faceT, faceW, faceH);

        auto info = RenderParam.GetFaceInfo(faceIndex);
        glm::mat4 matWorld;
        memcpy((float *)(&matWorld[0][0]), info->arrWMatrix, sizeof(float)*16);
        glm::mat4  matWVP = matProjDX*matWorld;
        for (int i = 0; i<m_vEffectModel.size(); ++i)
        {
            CCEffectModel *model = m_vEffectModel[i];
            if(model->m_bTransparent)
            {
                m_pShader->SetUniformMatrix4fv("u_matWVP",&matWVP[0][0],false,1);
                std::shared_ptr<MaterialTexRHI> pTex;
                if (model->m_drawable != NULL)
                {
                    pTex =model->m_drawable->GetTex(RenderParam.runtime);

                }
                if(pTex == NULL || pTex->GetWidth() == 0)
                {
                    continue;
                }
                pTex->Bind(0);
                m_pShader->SetUniformMatrix4fv("u_matWVP",&matWVP[0][0],false,1);
                m_pShader->SetVertexAttribPointer("a_position", 3, GL_FLOAT, false, 0, model->m_arrPos);
                m_pShader->SetVertexAttribPointer("a_inputTextureCoordinate", 2, GL_FLOAT, false, 0, model->m_arrUV);
                m_pShader->SetUniform4f("u_color",model->m_fMixColor.x,model->m_fMixColor.y,model->m_fMixColor.z,model->m_fMixColor.w);
				m_pShader->SetUniform4f("faceoriRect", faceOriRect.x, faceOriRect.y, faceOriRect.z, faceOriRect.w);
                glDrawElements(GL_TRIANGLES, model->m_nFaces * 3, GL_UNSIGNED_SHORT, model->m_arrIndex);
            }

        }
    }

	glViewport(0, 0, RenderParam.GetWidth(), RenderParam.GetHeight());
    glDisable(GL_DEPTH_TEST);
    if(m_v2DEffectModel.size()>0)
    {
        m_pShader2D->Use();

        for(int i=0; i<m_v2DEffectModel.size(); ++i)
        {
            CCEffect2DRect *rectEffect = m_v2DEffectModel[i];
            rectEffect->updateRenderInfo(width, height);
            std::shared_ptr<MaterialTexRHI> pTex;
            if (rectEffect->m_drawable != NULL)
            {
                pTex =rectEffect->m_drawable->GetTex(RenderParam.runtime);

            }
            pTex->Bind(0);
            m_pShader2D->SetVertexAttribPointer("a_position", 2, GL_FLOAT, false, 0, rectEffect->m_arrPos);
            m_pShader2D->SetVertexAttribPointer("a_texcoord", 2, GL_FLOAT, false, 0, rectEffect->m_arrUV);
            glDrawElements(GL_TRIANGLES, 2* 3, GL_UNSIGNED_SHORT, rectEffect->m_arrIndex);
        }
    }
	glColorMask(1, 1, 1, 1);
	glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
}

void CStickerEffectGL::Release()
{
    m_pShader.reset();
    m_pShaderHead.reset();
    m_pShader2D.reset();

    for (int i=0;i<m_vEffectModel.size();i++) {
        SAFE_DELETE(m_vEffectModel[i]);
    }
    for (int i=0;i<m_v2DEffectModel.size();i++) {
        SAFE_DELETE(m_v2DEffectModel[i]);
    }
    auto it = m_mapImage.begin();
    {
        while(it != m_mapImage.end())
        {
            if(it->second != NULL)
            {
                delete it->second;
            }
            ++it;
        }
    }
}

