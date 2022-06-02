#ifndef _CC3D_POST_PROCESS_MANAGER_H_
#define _CC3D_POST_PROCESS_MANAGER_H_

#include <memory>
#include <string>
#include"BaseDefine/Define.h"
#include "Toolbox/DXUtils/DX11DoubleBuffer.h"
#include "Toolbox/DXUtils/DX11Shader.h"
#include "Toolbox/DXUtils/DX11Texture.h"
#include "Toolbox/Render/CC3DShaderDef.h"
#include "Toolbox/Render/DynamicRHI.h"

#define  RECTATTRIBS 1
#define  VERT_NUM 4


#define DUAL_SAMPLE_COUNT 5
class PostProcessManager
{
public:
	PostProcessManager();
	~PostProcessManager();
	void Init(int nWidth, int nHeight);
	void Process();
	void SetShaderResource(const std::string& path);
	DX11Texture *m_EmissTexID;
	DX11DoubleBuffer *m_FBO_Ext;
	bool _enableFxaa = false;
	bool _enableBloom = false;
	float m_bloomAlpha = 0.0f; //暂时和选中的发光材质保持一致
	float m_bloomRadius = 1.0;
	int nIndex = 0;
	//TODO:还有数组类型的uniform没有解决，好像可以直接使用size去包含整个数组大小
	template<typename T>
	void SetParameter(std::string name, const T* value, size_t count, size_t size)
	{
		GET_SHADER_STRUCT_MEMBER(ConstantBufferVec4).SetParameter(name, value, size);
	}


private:
	DELCARE_SHADER_STRUCT_MEMBER(ConstantBufferVec4);

	void Fxaa(DX11Texture * input, int nWidth, int nHeight);
	void Bloom(DX11DoubleBuffer *pFBO);
	void DrawTriangle();

	DX11Shader* pFxaaShader = nullptr;
	DX11Shader* pScaleShader = nullptr;
	DX11Shader* pShader = nullptr;

	DX11Shader* pDualDownSample = nullptr;
	DX11Shader* pDualUpSample = nullptr;

	std::shared_ptr<CC3DVertexBuffer> mVertexBuffer;
	std::shared_ptr<CC3DIndexBuffer> mIndexBuffer;


	std::shared_ptr<CC3DRenderTargetRHI> m_BloomFBO ;
	std::array<std::shared_ptr<CC3DRenderTargetRHI>, DUAL_SAMPLE_COUNT> bloomLevelFBO;
	float m_scaleHW = 0.5;

	std::shared_ptr<CC3DBlendState> BlendADD;
	std::shared_ptr<CC3DBlendState> BlendADD2;
};

#endif