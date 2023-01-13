#ifndef _CC3D_POST_PROCESS_MANAGER_H_
#define _CC3D_POST_PROCESS_MANAGER_H_

#include "Toolbox/inc.h"
#include "BaseDefine/Define.h"
#include "Toolbox/Render/CC3DShaderDef.h"
#include "Toolbox/Render/DynamicRHI.h"

#define  RECTATTRIBS 1
#define  VERT_NUM 4


#define DUAL_SAMPLE_COUNT 5

class DoubleBufferRHI;

class PostProcessManager
{
public:
	PostProcessManager();
	~PostProcessManager();
	void Init(int nWidth, int nHeight);
	void Process();
	void SetShaderResource(const std::string& path);
	std::shared_ptr<DoubleBufferRHI> m_FBO_Ext;
	bool _enableFxaa = false;
	bool _enableBloom = false;
	float m_bloomAlpha = 0.0f; //暂时和选中的发光材质保持一致
	float m_bloomRadius = 1.0;
	int nIndex = 0;
	template<typename T>
	void SetParameter(std::string name, const T& value)
	{
		GET_SHADER_STRUCT_MEMBER(ConstantBufferVec4).SetParameter(name, value);
	}


private:
	DECLARE_SHADER_STRUCT_MEMBER(ConstantBufferVec4);

	void Bloom();
	void DrawTriangle();

	std::shared_ptr<ShaderRHI> pFxaaShader = nullptr;
	std::shared_ptr<ShaderRHI> pScaleShader = nullptr;
	std::shared_ptr<ShaderRHI> pShader = nullptr;

	std::shared_ptr<ShaderRHI> pDualDownSample = nullptr;
	std::shared_ptr<ShaderRHI> pDualUpSample = nullptr;

	std::shared_ptr<CC3DVertexBuffer> mVertexBuffer;
	std::shared_ptr<CC3DIndexBuffer> mIndexBuffer;


	std::shared_ptr<CC3DRenderTargetRHI> m_BloomFBO ;
	std::array<std::shared_ptr<CC3DRenderTargetRHI>, DUAL_SAMPLE_COUNT> bloomLevelFBO;
	float m_scaleHW = 0.5;

	std::shared_ptr<CC3DBlendState> BlendADD;
	std::shared_ptr<CC3DBlendState> BlendADD2;
};

#endif