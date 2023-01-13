#pragma once
#include "EffectKernel/CEffectPart.h"
#include "Scene/CC3DSceneManage.h"
#include "RenderGL/CC3DPbrRenderGL.h"
#include "CC3DEngine/Skeleton/CC3DSkeleton.h"
#include "Render/VertexBuffer.h"
#include "CC3DEngine/Render/CRenderPBRModelBase.h"


class CC3DSceneManage;
class CC3DPbrRenderGL;
class CC3DTexture;
class PostProcessManagerGL;
class CC3DExpressDrive;
class Drawable;

BEGIN_SHADER_STRUCT(LDRToHDR, 0)
	BEGIN_STRUCT_CONSTRUCT(LDRToHDR)
	    INIT_TEXTURE_INDEX("inputImageTexture", 0)
	END_SHADER_STRUCT
END_SHADER_STRUCT

BEGIN_SHADER_STRUCT(HDRToLDR, 0)
	BEGIN_STRUCT_CONSTRUCT(HDRToLDR)
	    INIT_TEXTURE_INDEX("inputImageTexture", 0)
	END_SHADER_STRUCT
END_SHADER_STRUCT
;


class CRenderPBRModelGL : public CRenderPBRModelBase
{
public:
	CRenderPBRModelGL();
	~CRenderPBRModelGL();

	bool Prepare(std::string &resourePath);
	void PreRender(BaseRenderParam& RenderParam);
	void Render(BaseRenderParam& RenderParam);
	void PostRender(BaseRenderParam& RenderParam);

	void Release();

	void InitBlendNameMap();
	void BeginDrawPBRModelToFbo();
	void ClearDepth();
	void DrawToColorBuffers(int nCount);

private:
	void CreatePostProcessManager() override;
private:

	std::shared_ptr<DoubleBufferRHI> mrtFbo;
	//CCProgram* pLDR2HDR = nullptr;
	//CCProgram* pHDR2LDR = nullptr;
	std::shared_ptr<CC3DVertexBuffer> HDRVerticeBuffer;
	std::shared_ptr<CC3DVertexBuffer> LDRVerticeBuffer;
	std::shared_ptr<CCProgram> pLDR2HDR;
	std::shared_ptr<CCProgram> pHDR2LDR;
	std::shared_ptr<CC3DTextureRHI> RenderSrcRHI;
	std::shared_ptr<CC3DTextureRHI> RenderPosRHI;


	CC3DPbrRenderGL *m_Render = nullptr;

	std::chrono::time_point<std::chrono::system_clock> lastTimeStamp;

	DECLARE_SHADER_STRUCT_MEMBER(LDRToHDR);
	DECLARE_SHADER_STRUCT_MEMBER(HDRToLDR);

	std::vector<std::string> m_BlendShapeName = {
	"jawForward", "jawLeft", "jawOpen", "jawRight" , "mouthClose", "mouthDimple_L","mouthDimple_R","mouthFrown_L", "mouthFrown_R",
	"mouthFunnel","mouthLeft","mouthLowerDown_L","mouthLowerDown_R","mouthPress_L","mouthPress_R","mouthPucker","mouthRight","mouthRollLower",
	"mouthRollUpper","mouthShrugLower","mouthShrugUpper","mouthSmile_L","mouthSmile_R","mouthStretch_L","mouthStretch_R","mouthUpperUp_L",
	"mouthUpperUp_R","browDown_L","browDown_R","browInnerUp","browOuterUp_L","browOuterUp_R","cheekPuff","cheekSquint_L","cheekSquint_R",
	"eyeBlink_L","eyeBlink_R","eyeLookDown_L","eyeLookDown_R","eyeLookIn_L","eyeLookIn_R","eyeLookOut_L","eyeLookOut_R","eyeLookUp_L","eyeLookUp_R",
	"eyeSquint_L","eyeSquint_R","eyeWide_L","eyeWide_R","noseSneer_L","noseSneer_R","tongueOut"
	};

	std::vector<std::string> m_BlendShapeName2 = {
	"jawForward", "jawLeft", "jawOpen", "jawRight" , "mouthClose", "mouthDimpleLeft","mouthDimpleRight","mouthFrownLeft", "mouthFrownRight",
	"mouthFunnel","mouthLeft","mouthLowerDownLeft","mouthLowerDownRight","mouthPressLeft","mouthPressRight","mouthPucker","mouthRight","mouthRollLower",
	"mouthRollUpper","mouthShrugLower","mouthShrugUpper","mouthSmileLeft","mouthSmileRight","mouthStretchLeft","mouthStretchRight","mouthUpperUpLeft",
	"mouthUpperUpRight","browDownLeft","browDownRight","browInnerUp","browOuterUpLeft","browOuterUpRight","cheekPuff","cheekSquintLeft","cheekSquintRight",
	"eyeBlinkLeft","eyeBlinkRight","eyeLookDownLeft","eyeLookDownRight","eyeLookInLeft","eyeLookInRight","eyeLookOutLeft","eyeLookOutRight","eyeLookUpLeft","eyeLookUpRight",
	"eyeSquintLeft","eyeSquintRight","eyeWideLeft","eyeWideRight","noseSneerLeft","noseSneerRight","tongueOut"
	};

	std::unordered_map<std::string, int> blendShapeMap;
	bool IsInit = false;
};