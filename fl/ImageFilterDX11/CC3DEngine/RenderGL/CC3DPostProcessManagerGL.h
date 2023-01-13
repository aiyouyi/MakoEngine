#ifndef _CC3D_POST_PROCESS_MANAGER_H_
#define _CC3D_POST_PROCESS_MANAGER_H_

#include <memory>
#include <string>
#include <vector>
#include"BaseDefine/Define.h"
#include <GL/CCProgram.h>
#include "ToolBox/GL/CCFramebuffer.h"
#include "ToolBox/GL/DoubleBuffer.h"
#include "Render/DynamicRHI.h"
#include "Render/VertexBuffer.h"
#include "Toolbox/Render/CC3DShaderDef.h"
#include "Render/TextureRHI.h"

#define  RECTATTRIBS 1
#define  VERT_NUM 3

BEGIN_SHADER_STRUCT(FXAA, 0)
	DECLARE_PARAM(Vector4, resolution)
	DECLARE_PARAM(Vector2, clipControl)
	BEGIN_STRUCT_CONSTRUCT(FXAA)
		IMPLEMENT_PARAM("resolution", UniformType::FLOAT4)
		IMPLEMENT_PARAM("clipControl", UniformType::FLOAT2)
		INIT_TEXTURE_INDEX("colorBuffer", 0)
	END_SHADER_STRUCT
END_SHADER_STRUCT

BEGIN_SHADER_STRUCT(Scale, 0)
	DECLARE_PARAM(Vector2, stepSize)
	BEGIN_STRUCT_CONSTRUCT(Scale)
		IMPLEMENT_PARAM("stepSize", UniformType::FLOAT2)
		INIT_TEXTURE_INDEX("inputImageTexture", 0)
	END_SHADER_STRUCT
END_SHADER_STRUCT

BEGIN_SHADER_STRUCT(DualDowmSample, 0)
	DECLARE_PARAM(Vector2, mainTex_texelSize)
	DECLARE_PARAM(float, _Offset)
	BEGIN_STRUCT_CONSTRUCT(DualDowmSample)
		IMPLEMENT_PARAM("mainTex_texelSize", UniformType::FLOAT2)
		IMPLEMENT_PARAM("_Offset", UniformType::FLOAT)
		INIT_TEXTURE_INDEX("MainTex", 0)
	END_SHADER_STRUCT
END_SHADER_STRUCT

BEGIN_SHADER_STRUCT(DualUpSample, 0)
	DECLARE_PARAM(Vector2, mainTex_texelSize)
	DECLARE_PARAM(float, _Offset)
	BEGIN_STRUCT_CONSTRUCT(DualUpSample)
		IMPLEMENT_PARAM("mainTex_texelSize", UniformType::FLOAT2)
		IMPLEMENT_PARAM("_Offset", UniformType::FLOAT)
	INIT_TEXTURE_INDEX("MainTex", 0)
	END_SHADER_STRUCT
END_SHADER_STRUCT

BEGIN_SHADER_STRUCT(Quad2D, 0)
	DECLARE_PARAM(float, alpha)
	BEGIN_STRUCT_CONSTRUCT(Quad2D)
		IMPLEMENT_PARAM("alpha", UniformType::FLOAT)
		INIT_TEXTURE_INDEX("inputImageTexture", 0)
	END_SHADER_STRUCT
END_SHADER_STRUCT
;

class PostProcessManagerGL
{
public:
	PostProcessManagerGL();
	~PostProcessManagerGL();
	void Init(int nWidth, int nHeight);
	uint32 Process();
	void SetShaderResource(const std::string& path);

	CCFrameBuffer *m_FBO_Ext;
	bool _enableFxaa = false;
	bool _enableBloom = false;
	float m_bloomAlpha = 0.0f; //暂时和选中的发光材质保持一致
	float m_bloomRadius = 1.0;
private:

	void Fxaa(uint32 input, int nWidth, int nHeight);
	GLuint Bloom(CCFrameBuffer* pFBO);
	void DrawTriangle();

	void InitFrameBuffer(int nWidth, int nHeight);


	std::shared_ptr<CCProgram> pFxaaShader;
	std::shared_ptr<CCProgram> pScaleShader;
	std::shared_ptr<CCProgram> pBulrShader;
	std::shared_ptr<CCProgram> pShader;

	std::shared_ptr<CCProgram> pDualDownSample;
	std::shared_ptr<CCProgram> pDualUpSample;

	//bloom 各个层级
	std::vector<CCFrameBuffer*> bloomLevelFBO;

	//GLuint mEbo;
	//GLuint mVbo[RECTATTRIBS];
	//GLuint mVao;
	//用来做fxaa
	CCFrameBuffer *m_FullSizeBuffer;
	//用来做bloom
	CCFrameBuffer *m_HalfSizeBuffer;
	float m_scaleHW = 0.5;

	int _Width = 0;
	int _Height = 0;

	std::shared_ptr<CC3DVertexBuffer> postProcessTriMesh;
	std::shared_ptr<CC3DIndexBuffer> postProcessIndexBuffer;
	std::shared_ptr<CC3DTextureRHI> InputTextureRHI;

	DECLARE_SHADER_STRUCT_MEMBER(FXAA);
	DECLARE_SHADER_STRUCT_MEMBER(Scale);
	DECLARE_SHADER_STRUCT_MEMBER(DualDowmSample);
	DECLARE_SHADER_STRUCT_MEMBER(DualUpSample);
	DECLARE_SHADER_STRUCT_MEMBER(Quad2D);
};

#endif