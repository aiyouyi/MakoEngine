#include "CFaceEffect3DNormal.h"
#include "BaseDefine/Define.h"
#include "BaseDefine/Vectors.h"
#include "EffectKernel/ShaderProgramManager.h"
#include "Toolbox/Render/TextureRHI.h"
#include "Toolbox/Render/DynamicRHI.h"
#include "Toolbox/Render/VertexBuffer.h"
#include "Toolbox/RenderState/PiplelineState.h"


CFaceEffect3DNormal::CFaceEffect3DNormal()
{
	m_isFirstInit = true;

}


CFaceEffect3DNormal::~CFaceEffect3DNormal()
{
	Release();
}

void CFaceEffect3DNormal::Release()
{
}

void * CFaceEffect3DNormal::Clone()
{
	CFaceEffect3DNormal* result = new CFaceEffect3DNormal();
	*result = *this;
	return result;
}

bool CFaceEffect3DNormal::ReadConfig(XMLNode & childNode, HZIP hZip ,char *pFilePath)
{
	if (!childNode.isEmpty())
	{
		XMLNode nodeDrawable = childNode.getChildNode("drawable", 0);
		if (!nodeDrawable.isEmpty())
		{
			const char *szDrawableName = nodeDrawable.getAttribute("Normal");
			if (szDrawableName != NULL)
			{
				m_NormalMap = GetDynamicRHI()->CreateTextureFromZip(hZip, szDrawableName, false);
			}
		}
		return true;
	}
	return false;
}

bool CFaceEffect3DNormal::Prepare()
{
	//´´½¨shader
	CCVetexAttribute pAttribute[] =
	{
		{VERTEX_ATTRIB_POSITION, FLOAT_C3},
		{VERTEX_ATTRIB_TEX_COORD, FLOAT_C2},
	};

	std::string path = m_resourcePath + "/Shader/face3DNormal.fx";
	m_pShader = ShaderProgramManager::GetInstance()->GetOrCreateShaderByPathAndAttribs(path, pAttribute, 2,true);

	if (m_Reconstruct3D == NULL)
	{
		m_Reconstruct3D = new mt3dface::MultiLinearVideoReconstructor();

		string path = m_resourcePath + "/3DModels/";
		m_Reconstruct3D->LoadModel(path.c_str());
	}
	return true;
}

void CFaceEffect3DNormal::Render(BaseRenderParam &RenderParam)
{
	if (RenderParam.GetFaceCount() == 0)
	{
		m_isFirstInit = true;
		return;
	}
	long runTime = GetRunTime();
	if (runTime < 0)
	{
		return;
	}
	auto pDoubleBuffer = RenderParam.GetDoubleBuffer();
	pDoubleBuffer->BindFBOA();

	GetDynamicRHI()->SetRasterizerState(CC3DPiplelineState::RasterizerStateCullBack);

	pDoubleBuffer->GetFBOA()->clearDepth();
	GetDynamicRHI()->SetDepthStencilState(CC3DPiplelineState::DepthStateEnable, 0);


	m_pShader->useShader();

	auto FaceInfo = RenderParam.GetFaceInfo(0);;
	auto pFacePoint = RenderParam.GetFacePoint(0);
	m_Reconstruct3D->Run((float*)pFacePoint, RenderParam.GetWidth(), RenderParam.GetHeight(), FaceInfo->faceW , FaceInfo->faceH, m_isFirstInit, 2, 2);
	m_isFirstInit = false;
	m_Reconstruct3D->OrthoToPerspectMVP(20.0, 1);

	if (!VerticeBuffer[0])
	{
		VerticeBuffer[0] = GetDynamicRHI()->CreateVertexBuffer(m_Reconstruct3D->m_pAdjustModel3D, MLM_FACE_3D_UV_VERTEX, 3);
	}
	else
	{
		VerticeBuffer[0]->UpdateVertexBUffer(m_Reconstruct3D->m_pAdjustModel3D, MLM_FACE_3D_UV_VERTEX, 3 * sizeof(float), 3 * sizeof(float));
	}

	if (!VerticeBuffer[1])
	{
		VerticeBuffer[1] = GetDynamicRHI()->CreateVertexBuffer(m_Reconstruct3D->model->GetTextureCoordinates(), MLM_FACE_3D_UV_VERTEX, 2);
	}
	if (!IndexBuffer)
	{
		IndexBuffer = GetDynamicRHI()->CreateIndexBuffer(m_Reconstruct3D->model->GetTriangleIndex(), MLM_TRIANGLE_WITH_LIPS);
	}
	
	glm::mat4 mWVP;
	memcpy(&mWVP[0][0], m_Reconstruct3D->m_fModelViewPerspect, 16 * sizeof(float));
	mWVP = glm::transpose(mWVP);

	auto cam_params = m_Reconstruct3D->m_pCameraParamPerspect[MLVR_Smooth3DFace];

	glm::mat4 rot_mtx_x = glm::rotate(glm::mat4(1.0f), -cam_params[0]*3.1416f / 180.0f, glm::vec3{ 1.0f, 0.0f, 0.0f });
	glm::mat4 rot_mtx_y = glm::rotate(glm::mat4(1.0f), -cam_params[1] *3.1416f / 180.0f, glm::vec3{ 0.0f, 1.0f, 0.0f });
	glm::mat4 rot_mtx_z = glm::rotate(glm::mat4(1.0f), cam_params[2] *3.1416f / 180.0f, glm::vec3{ 0.0f, 0.0f, 1.0f });

	glm::mat4 NormalMat = rot_mtx_z * rot_mtx_x*rot_mtx_y;

	SetParameter("matWVP", &mWVP, 0, sizeof(glm::mat4));
	SetParameter("matNormal", &NormalMat, 0, sizeof(glm::mat4));

	GET_SHADER_STRUCT_MEMBER(NormalConstantBuffer).ApplyToAllBuffer();

	GetDynamicRHI()->SetPSShaderResource(0, m_NormalMap);

	GetDynamicRHI()->DrawPrimitive(VerticeBuffer,2, IndexBuffer);

	GetDynamicRHI()->SetDepthStencilState(CC3DPiplelineState::DepthStateDisable);
	GetDynamicRHI()->SetRasterizerState(CC3DPiplelineState::RasterizerStateCullNone);
}

