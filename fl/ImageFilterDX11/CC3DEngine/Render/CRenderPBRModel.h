#pragma once
#include "EffectKernel/CEffectPart.h"
#include "CRenderPBRModelBase.h"
//#include "Toolbox/EffectModel.hpp"

namespace CC3DImageFilter
{
	class CC3DSceneManage;
	class CC3DPbrRender;
}

class CC3DTexture;
class PostProcessManager;
class CC3DExpressDrive;
class DoubleBufferRHI;

class CRenderPBRModel : public CRenderPBRModelBase
{
public:
	CRenderPBRModel();
	~CRenderPBRModel();
	bool Prepare(const std::string &resourePath);
	void PreRender(BaseRenderParam& RenderParam);
	void Render(BaseRenderParam& RenderParam);
	void PostRender(BaseRenderParam& RenderParam);
	void Release();
	std::shared_ptr<DoubleBufferRHI> GetDoubleBuffer() { return m_DoubleBuffer; }
	void SetHDRPath(const std::string& path);

	void CreatePostProcessManager() override;

	long RunTime = 0;
	bool RenderBground = false;
private:
	
	std::shared_ptr<DoubleBufferRHI> m_DoubleBuffer;
	std::shared_ptr<ShaderRHI> pLDR2HDR;
	std::shared_ptr<ShaderRHI> pHDR2LDR;

	std::shared_ptr<CC3DVertexBuffer> mVertexBuffer;
	std::shared_ptr<CC3DIndexBuffer> mIndexBuffer;
	
	std::shared_ptr<CC3DImageFilter::CC3DPbrRender> m_Render = nullptr;

	std::chrono::time_point<std::chrono::system_clock> lastTimeStamp;
	bool IsInit = false;
};

