#include "Toolbox/DXUtils/DXUtils.h"
#include "CBackGroundFilter.h"
#include "BaseDefine/Define.h"
#include "EffectKernel/ShaderProgramManager.h"
#include "EffectKernel/ResourceManager.h"
#include "EffectKernel/FileManager.h"
#include "Toolbox/DXUtils/DX11Resource.h"
#include "Toolbox/Render/DynamicRHI.h"
#include "Toolbox/Render/VertexBuffer.h"
#include "Toolbox/RenderState/PiplelineState.h"

CBackGroundFilter::CBackGroundFilter()
{
	m_EffectPart = BACK_GROUND_FILTER;

	m_alpha = 1.0;
}

CBackGroundFilter::~CBackGroundFilter()
{
	Release();
}

void CBackGroundFilter::Release()
{

}

void * CBackGroundFilter::Clone()
{
	CBackGroundFilter* result = new CBackGroundFilter();
	*result = *this;
	return result;
}

bool CBackGroundFilter::ReadConfig(XMLNode & childNode, HZIP hZip ,char *pFilePath)
{
	if (!childNode.isEmpty())
	{
		XMLNode nodeDrawable = childNode.getChildNode("drawable", 0);
		if (!nodeDrawable.isEmpty())
		{
			const char* szAlpha = nodeDrawable.getAttribute("Alpha");
			if (szAlpha != NULL)
			{
				m_alpha = atof(szAlpha);
			}
			const char* szRadius = nodeDrawable.getAttribute("FilterRadis");
			if (szRadius != NULL)
			{
				m_Radius = atof(szRadius);
			}

			return true;
		}

		//Time
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
		return true;
	}
	return false;



}

bool CBackGroundFilter::Prepare()
{
	m_Shader = GetDynamicRHI()->CreateShaderRHI();
	if (GetDynamicRHI()->API == CC3DDynamicRHI::DX11)
	{
		CCVetexAttribute pAttribute[] =
		{
			{VERTEX_ATTRIB_POSITION, FLOAT_C2},
		};

		std::string fsPath = m_resourcePath + "/Shader/backGroundFilter.fx";
		m_Shader->InitShader(fsPath, pAttribute, 1, false);
	}
	else
	{
		//To do GL
	}

	m_HeadManage = std::make_shared<HeadManage>();

	m_nVerts = g_TextureCoordinate.size() / 2;

	if (m_VerticeBuffer == NULL)
	{
		m_VerticeBuffer = GetDynamicRHI()->CreateVertexBuffer(&g_TextureCoordinate[0], m_nVerts, 2);
	}
	if (m_IndexBuffer == NULL)
	{
		unsigned short index[] =
		{
			0, 1, 2,
			1, 2, 3
		};

		m_IndexBuffer = GetDynamicRHI()->CreateIndexBuffer(index, 2);
	}
	return true;
}

void CBackGroundFilter::Render(BaseRenderParam &RenderParam)
{
	if (m_alpha < 0.001f)
	{
		return;
	}
	if (RenderParam.GetFaceCount() == 0)
	{
		return;
	}


	auto pDoubleBuffer = RenderParam.GetDoubleBuffer();

	Vector2* pFacePoint = RenderParam.GetFacePoint(0, FACE_POINT_106);
	m_HeadManage->GenHeadMask(pFacePoint, RenderParam.GetWidth(), RenderParam.GetHeight());
	m_FaceMask = GetDynamicRHI()->CreateTexture(CC3DTextureRHI::SFT_A8R8G8B8, CC3DTextureRHI::OT_NONE, m_HeadManage->m_nMaskWidth, m_HeadManage->m_nMaskHeight,(BYTE*)m_HeadManage->m_pHeadMask, m_HeadManage->m_nMaskWidth * 4, false);

	pDoubleBuffer->BindFBOB();
	m_Shader->UseShader();
	
	GetDynamicRHI()->SetPSShaderResource(0, RHIResourceCast(pDoubleBuffer.get())->GetFBOTextureA());
	GetDynamicRHI()->SetPSShaderResource(1, m_FaceMask);
	
	Vector4 pParam;
	pParam.x = m_alpha;
	pParam.y = 0.0;
	pParam.z = m_Radius / RenderParam.GetHeight();

	GET_SHADER_STRUCT_MEMBER(GaussianBlur).SetParameter("pParam", pParam);
	GET_SHADER_STRUCT_MEMBER(GaussianBlur).ApplyToAllBuffer();

	GetDynamicRHI()->DrawPrimitive(m_VerticeBuffer,m_IndexBuffer);

	
	pDoubleBuffer->BindFBOA();
	m_Shader->UseShader();

	GetDynamicRHI()->SetPSShaderResource(0, RHIResourceCast(pDoubleBuffer.get())->GetFBOTextureB());
	GetDynamicRHI()->SetPSShaderResource(1, m_FaceMask);

	pParam.x = m_alpha;
	pParam.y = m_Radius / RenderParam.GetWidth();
	pParam.z = 0.0;

	GET_SHADER_STRUCT_MEMBER(GaussianBlur).SetParameter("pParam", pParam);
	GET_SHADER_STRUCT_MEMBER(GaussianBlur).ApplyToAllBuffer();

	GetDynamicRHI()->DrawPrimitive(m_VerticeBuffer, m_IndexBuffer);
}


