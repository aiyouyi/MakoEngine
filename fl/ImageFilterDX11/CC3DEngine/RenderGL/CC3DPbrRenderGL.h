#pragma once
#ifndef _H_CC3D_PBRRENDER_H_
#define _H_CC3D_PBRRENDER_H_

#include "CC3DEngine/Scene/CC3DSceneManage.h"
#include "GL/CCProgram.h"
#include "GL/MSAAFramebuffer.h"
#include "CC3DRenderCubeGL.h"
#include "Toolbox/Render/CC3DShaderDef.h"
#include "Material/CC3DMaterial.h"
#include "RenderCommon/PBRRenderDef.h"
#include "CC3DEngine/CC3DConfigStream.h"


class CC3DShadowRender;
namespace CC3DImageFilter
{
	class ShadowMapManager;
}

BEGIN_SHADER_STRUCT(RenderBackGround,0)
	DECLARE_PARAM(glm::mat4, projection)
		BEGIN_STRUCT_CONSTRUCT(RenderBackGround)
		IMPLEMENT_PARAM("projection", UniformType::MAT4)
		IMPLEMENT_PARAM("view", UniformType::MAT4)
		INIT_TEXTURE_INDEX("environmentMap", 0)
	END_SHADER_STRUCT
END_SHADER_STRUCT
;
class CC3DPbrRenderGL
{
public:
	CC3DPbrRenderGL(void);
	~CC3DPbrRenderGL(void);

	void InitSecne(CC3DImageFilter::CC3DSceneManage *Scene) { m_3DSceneManage = Scene; }
	void SetShaderResource(std::string path);
	//设置背景清除颜色
	void SetClearColor(float r, float g, float b, float a);

	void update();
	void Render(uint32 nWidth, uint32 nHeight);
	void RenderCube();
	void OutRenderShadowMap();
	void SetHDRPath(std::string fileName);
	void SetHDRData(float *pData, int &nWidth, int &nHeight);

	void RenderShadowMap();

	void RenderBackGround();
	//离屏渲染获取渲染结果纹理ID
	 uint32 GetRenderResultTextureID();

	 void ResetUVAnimation();

	 void SetModeConfig(CC3DImageFilter::ModelConfig* _ModelConfig);
	
	 CCFrameBuffer* m_FrameBuffer;
	 MSAAFramebuffer * m_MsaaFrameBuffer;
protected:
	//背景颜色

	//void RenderWithSkin(CC3DMesh *pMesh,int nSkeleton);
	//void RenderWithOutSkin(CC3DMesh *pMesh);

	void SetAnimation(CC3DMesh* pMesh, int nSkeleton);
	void RenderMesh(CC3DMesh* pMesh, bool withAnimation);

	void PreRenderJob();
	void PreRenderMesh(CC3DMesh* pMesh, bool withAnimation);
	void PreRenderSet(CC3DMesh* pMesh);
	void RenderJob();

	void RenderSet(CC3DMesh *pMesh);

		//设置渲染大小
	void SetSize(uint32 nWidth, uint32 nHeight);

	void SetMatrix(CC3DMaterial* mtl);
	void SetLight(CC3DMaterial* mtl);
	void SetCommon();
	//对Mesh进行排序，按顺序渲染
	void GetSortMeshID();
	void GetBoxPoint(Vector3& minPoint, Vector3& maxPoint);

	glm::vec4 m_ClearColor;
	//渲染屏幕大小
	uint32 m_nRenderWidth;
	uint32 m_nRenderHeight;


	std::string m_resourcePath;
	CC3DImageFilter::CC3DSceneManage *m_3DSceneManage;
	CC3DRenderCubeGL *m_RenderCube;
	std::shared_ptr<CC3DShadowRender> _shadowRender;
	std::shared_ptr<CC3DImageFilter::ShadowMapManager> _shadowMapManager;

	std::shared_ptr<CCProgram> m_pShaderBack;

	std::vector<CC3DImageFilter::MeshDisInfo> m_SortMesh;
	std::vector<Vector3> m_BoxPoint;
	bool m_NeedRenderCube = true;

	std::map<MaterialType, int> m_MaterialType;

	float _timeElapse_glass = 0.0f;
	float _timeElapse_wave = 0.0f;
	float _timeElapse_particle = 0.0f;
	float uv_rotate_time = 0.0f;

	DECLARE_SHADER_STRUCT_MEMBER(RenderBackGround);

	CC3DImageFilter::ModelConfig* Config = nullptr;
};


#endif // _H_CC3D_PBRRENDER_H_
