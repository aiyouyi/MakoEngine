#include "CFaceShadowHighLightGL.h"
#include "BaseDefine/Define.h"
#include "BaseDefine/Vectors.h"
#include "BaseDefine/commonFunc.h"
#include "Toolbox/GL/CCProgram.h"
#include "Toolbox/GL/GLResource.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "Toolbox/Render/DynamicRHI.h"
#include "Toolbox/Render/MaterialTexRHI.h"

CFaceShadowHighLightGL::CFaceShadowHighLightGL()
{
	m_2DInterFace = NULL;
    m_alpha = 0.0;
    m_EffectPart = SHADOW_HIGHLIGHT_EFFECT;
}


CFaceShadowHighLightGL::~CFaceShadowHighLightGL()
{
	Release();
}

void CFaceShadowHighLightGL::Release()
{
   // GL_DELETE_TEXTURE(m_material);
	SAFE_DELETE(m_2DInterFace);
}

void * CFaceShadowHighLightGL::Clone()
{
	CFaceShadowHighLightGL* result = new CFaceShadowHighLightGL();
	*result = *this;
	return result;
}

bool CFaceShadowHighLightGL::ReadConfig(XMLNode & childNode, HZIP hZip ,char *pFilePath)
{
	if (!childNode.isEmpty())
	{
		XMLNode nodeDrawable = childNode.getChildNode("drawable", 0);
		if (!nodeDrawable.isEmpty())
		{
			const char *szDrawableName = nodeDrawable.getAttribute("Normal");
			const char *szmask = nodeDrawable.getAttribute("Mask");

			const char *szHL= nodeDrawable.getAttribute("Hightlight");
			const char *szSW = nodeDrawable.getAttribute("Shadow");
			if (szDrawableName != NULL)
			{
				m_Normal = GetDynamicRHI()->CreateAsynTextureZIP(hZip, szDrawableName);
			}
			if (szmask != NULL)
			{
				m_mask = GetDynamicRHI()->CreateAsynTextureZIP(hZip, szmask);
			}

			if (szHL != NULL)
			{
				m_Highlight = GetDynamicRHI()->CreateAsynTextureZIP(hZip, szHL);
			}
			if (szSW != NULL)
			{
				m_Shadowdow = GetDynamicRHI()->CreateAsynTextureZIP(hZip, szSW);
			}
		}
		return true;
	}
	return false;
}

bool CFaceShadowHighLightGL::Prepare()
{
	m_pShader = std::make_shared<CCProgram>();
    std::string  vspath = m_resourcePath + "/Shader/faceHightLight.vs";
    std::string  fspath = m_resourcePath + "/Shader/faceHightLight.fs";
    m_pShader->CreatePorgramForFile(vspath.c_str(),fspath.c_str());
	m_2DInterFace = new mt3dface::MTFace2DInterFace();
	return true;
}

void CFaceShadowHighLightGL::Render(BaseRenderParam& RenderParam)
{
    if(m_alpha<0.001f)
    {
        return;
    }

	if (RenderParam.GetFaceCount() == 0)
	{
		return;
	}

    auto pDoubleBuffer = RenderParam.GetDoubleBuffer();
	int nFaceCount = RenderParam.GetFaceCount();
   // pDoubleBuffer->SyncAToB();
    
//     if (isFirst) {
//         isFirst = false;
//         pDoubleBuffer->BindFBOB();
//     }
//     else
//     {
//         pDoubleBuffer->BindFBOA();
//     }
    
    m_pShader->Use();
    glViewport(0,0,RenderParam.GetWidth(), RenderParam.GetHeight());


	for (int faceIndex = 0; faceIndex < nFaceCount; faceIndex++)
    {
		Vector2 *pFacePoint = RenderParam.GetFacePoint(faceIndex, FACE_POINT_106);

		auto info = RenderParam.GetFaceInfo(faceIndex);
		glm::mat4 rot_mtx_x = glm::rotate(glm::mat4(1.0f), (float)info->pitch*3.1416f/180.0f, glm::vec3{ 1.0f, 0.0f, 0.0f });
		glm::mat4 rot_mtx_y = glm::rotate(glm::mat4(1.0f), (float)info->yaw*3.1416f / 180.0f, glm::vec3{ 0.0f, 1.0f, 0.0f });
		glm::mat4 rot_mtx_z = glm::rotate(glm::mat4(1.0f), (float)info->roll*3.1416f / 180.0f, glm::vec3{ 0.0f, 0.0f, 1.0f });

		glm::mat4 rot_mtx = rot_mtx_z * rot_mtx_x*rot_mtx_y;
		rot_mtx = glm::transpose(rot_mtx);

		m_pShader->SetUniform3f("cameraZRotation", rot_mtx[2][0], rot_mtx[2][1], rot_mtx[2][2]);
		auto FaceMesh = m_2DInterFace->Get2DMesh((float*)pFacePoint, RenderParam.GetWidth(), RenderParam.GetHeight(), 0, mt3dface::MT_FACE_25D);
		pDoubleBuffer->SyncAToBRegion((float*)FaceMesh->pVertexs, FaceMesh->nVertex, 3, 1);

		pDoubleBuffer->BindFBOA();

		m_mask->Bind(1);
		m_pShader->SetUniform1i("inputImageTexture2", 1);
		m_Normal->Bind(2);
		m_pShader->SetUniform1i("shNormalTexture", 2);
		m_Highlight->Bind(3);
		m_pShader->SetUniform1i("hightLightLookupTexture", 3);
		m_Shadowdow->Bind(4);
		m_pShader->SetUniform1i("shadowLookupTexture", 4);
		m_pShader->SetUniform1f("alpha", m_alpha);


		GetDynamicRHI()->SetPSShaderResource(0, pDoubleBuffer->GetFBOTextureB());
		m_pShader->SetUniform1i("inputImageTexture", 0);

		float *pMaskUV = m_2DInterFace->GetStandVerts();      
        m_pShader->SetVertexAttribPointer("a_position", 3, GL_FLOAT, false, 0, (float*)FaceMesh->pVertexs);
        m_pShader->SetVertexAttribPointer("a_texcoord", 2, GL_FLOAT, false, 0, (float*)FaceMesh->pTextureCoordinates);
        glDrawElements(GL_TRIANGLES, FaceMesh->nTriangle*3, GL_UNSIGNED_SHORT, FaceMesh->pTriangleIndex);
	}
    
    
}
