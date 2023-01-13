#pragma once
#ifndef _H_CC3D_PBRRENDER_H_
#define _H_CC3D_PBRRENDER_H_

#include <map>

#include "CC3DEngine/Scene/CC3DSceneManage.h"
#include "CC3DEngine/Render/CC3DRenderCube.h"
#include "ToolBox/DXUtils/DX11Shader.h"
#include "RenderCommon/PBRRenderDef.h"
#include "Material/EffectConfig.h"

class CC3DShadowRender;

namespace CC3DImageFilter
{	
	class ShadowMapManager;

	class CC3DPbrRender
	{
	public:
		CC3DPbrRender(void);
		~CC3DPbrRender(void);

		void InitSecne(CC3DSceneManage* Scene) { m_3DSceneManage = Scene; }
		void SetShaderResource(const std::string& path);
		//设置背景清除颜色
		void SetClearColor(float r, float g, float b, float a);

		void Render(uint32 nWidth, uint32 nHeight);
		void RenderCube();
		void SetHDRPath(const std::string& fileName);

		void SetCommonUniform();
		void RenderBackGround();

		void SetModeConfig(CC3DImageFilter::EffectConfig* EffectConfig);
		void RenderShadowMap();

	public:
		CC3DRenderCube* m_RenderCube;
	protected:

		void PreRenderJob();
		void PreRenderMesh(CC3DMesh* pMesh, bool withAnimation);
		void PreRenderSet(CC3DMesh* pMesh);

		void RenderJob();
		void RenderMesh(CC3DMesh* pMesh, bool withAnimation);
		void RenderSet(CC3DMesh* pMesh);

		void SetAnimation(CC3DMesh* pMesh, int nSkeleton);

		//设置渲染大小
		void SetSize(uint32 nWidth, uint32 nHeight);

		void SetMatrix(CC3DMaterial* pMaterial);

		void SetLight(CC3DMaterial* pMaterial);
		//对Mesh进行排序，按顺序渲染
		void GetSortMeshID();
		void GetBoxPoint(Vector3& minPoint, Vector3& maxPoint);
		void RenderOutline();


		glm::vec4 m_ClearColor;
		//渲染屏幕大小
		uint32 m_nRenderWidth;
		uint32 m_nRenderHeight;

		std::string m_resourcePath;
		//DX11Shader *m_pShaderPBR;
		//DX11Shader *m_pShaderSkinPBR;
		CC3DSceneManage* m_3DSceneManage;


		DX11Shader* m_pShaderBack;

		bool m_NeedRenderCube = true;

		std::shared_ptr<CC3DDepthStencilState> m_pDepthStateEnableWriteDisable;
		std::shared_ptr<CC3DDepthStencilState> m_pDepthStateEnableWriteEnable;
		std::shared_ptr<CC3DDepthStencilState> m_OutlineDepthState;

		std::vector<CC3DImageFilter::MeshDisInfo> m_SortMesh;
		std::vector<Vector3> m_BoxPoint;

		std::shared_ptr< ShadowMapManager> m_ShadowMapMgr;
		std::shared_ptr<CC3DShadowRender> m_ShadowRender;

		IBLConstantBuffer m_IBLConstantBuffer;
		std::shared_ptr<CC3DConstantBuffer> m_IBLCB;

		CC3DImageFilter::SkinInfo SkinInfoParam;

		CC3DImageFilter::EffectConfig* EffectConfig = nullptr;
	};
}



#endif // _H_CC3D_PBRRENDER_H_
