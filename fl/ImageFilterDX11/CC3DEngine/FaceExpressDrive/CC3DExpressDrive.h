#pragma once
#ifndef _H_CC3D_EXPRESSDRIVE_H_
#define _H_CC3D_EXPRESSDRIVE_H_
#include "BaseDefine/Vectors.h"
#include "BaseDefine/Define.h"
//#include "FacialAnalyser/ExpressionAnalyserAPI.h"
#include "FacialAnalyser/ExpressionAnalyserAPI_C.h"
#include <vector>
#include <string>
#include <unordered_map>
#include "DX11ImageFilterDef.h"

#define BlendShapeCoffes 52

#define TR_HEADPOSE_COUNT  6
#define TR_EYEGAZE_COUNT  6
#define TR_AU_COUNT  74


struct TrackingResult
{
	//The headPose format is[Tx, Ty, Tz, Eul_x, Eul_y, Eul_z]
	float headPose[TR_HEADPOSE_COUNT];
	float eyeGazeDir[TR_EYEGAZE_COUNT];
	std::pair<int, float> au[TR_AU_COUNT];
	bool isRecording;
	bool isPlaying;
};


enum  CC3DFilterType 
{
	CC3DMEAN,        // index, length(Radius + 1), 
	CC3DGAUSS,       // index, length(Radius + 1), SigmaSpace
	CC3DBILATERAL    // index, length(Radius + 1), SigmaSpace, mSigmaValue
};

enum  CC3DCurveType
{
	CC3DNORMAL,

	CC3DEASE_IN_SINE,
	CC3DEASE_OUT_SINE,
	CC3DEASE_IN_OUT_SINE,
	CC3DEASE_IN_QUAD,
	CC3DEASE_OUT_QUAD,
	CC3DEASE_IN_OUT_QUAD,

	CC3DEASE_IN_CUBIC,
	CC3DEASE_OUT_CUBIC,
	CC3DEASE_IN_OUT_CUBIC,
	CC3DEASE_IN_QUART,
	CC3DEASE_OUT_QUART,
	CC3DEASE_IN_OUT_QUART,

	CC3DEASE_IN_QUINT,
	CC3DEASE_OUT_QUINT,
	CC3DEASE_IN_OUT_QUINT,
	CC3DEASE_IN_EXPO,
	CC3DEASE_OUT_EXPO,
	CC3DEASE_IN_OUT_EXPO,

	CC3DEASE_IN_CIRC,
	CC3DEASE_OUT_CIRC,
	CC3DEASE_IN_OUT_CIRC,
	CC3DEASE_IN_BACK,
	CC3DEASE_OUT_BACK,
	CC3DEASE_IN_OUT_BACK,

	CC3DEASE_IN_ELASTIC,
	CC3DEASE_OUT_ELASTIC,
	CC3DEASE_IN_OUT_ELASTIC,
	CC3DEASE_IN_BOUNCE,
	CC3DEASE_OUT_BOUNCE,
	CC3DEASE_IN_OUT_BOUNCE,
};

enum  CC3DEffectType
{
	CC3DDEACTIVE,     // index
	CC3DCLAMP,        // index, min, max
	CC3DOFFSET,       // index, offset
	CC3DCURVE,        // index, CurveType
	CC3DINCREASE_BY,  // target index, source index, CurveType, scale
	CC3DREDUCE_BY,    // target index, source index, CurveType, scale
	CC3DMIRROR,       // index1, index2, weight1, weight2
};

enum  CC3DModifyType
{
	CC3DEFFECT,
	CC3DFILTER,
};

// enum  ParamType
// {
// 	CURVE,
// 	NUMBER
// };

struct CC3DEffectData
{
	bool                mIsActive;
	std::string         mEffectName;
	std::vector<int>    mIndices;
	std::vector<double> mParams;
};

class CC3DFilter
{
public:
	CC3DFilter(CC3DFilterType pType, const CC3DEffectData& pEffectData) :
		mType(pType),
		mValues((int)pEffectData.mParams[0], 0),
		mStart(0),
		mLength((int)pEffectData.mParams[0]),
		mPrecpt()
	{
		mEffectData = (CC3DEffectData*)&pEffectData;
		switch (mType)
		{
		case CC3DFilterType::CC3DMEAN:
			break;
		case CC3DFilterType::CC3DGAUSS:
		case CC3DFilterType::CC3DBILATERAL:
		{
			// length(Radius + 1), SigmaSpace, mSigmaValue
			double lCoeffSpace = -0.5 / (pEffectData.mParams[1] * pEffectData.mParams[1]);

			for (int i = 1 - mLength; i <= 0; ++i)
			{
				mPrecpt.push_back(exp(lCoeffSpace * i * i));
			}
			break;
		}
		default:
			break;
		}
	}

	void process(TrackingResult& pTr);

private:
	CC3DFilterType         mType;
	CC3DEffectData *mEffectData;
	std::vector<double> mValues;
	int mStart;
	int mLength;
	std::vector<double> mPrecpt;
};
#endif // _H_CC3D_EXPRESSDRIVE_H_


class DX11IMAGEFILTER_EXPORTS_CLASS CC3DExpressDrive
{
public:
	CC3DExpressDrive();
	virtual ~CC3DExpressDrive();

	void SetModelPath(const char* modelPath);
	void GetBlendshapeWeights(unsigned char *data, int width, int height, Vector2 * faceRes);
	void FacePointIndexTransfer(Vector2 * pSrcPoint130, std::vector<float>& mDstPoint74);
	void PostProcess();
	std::vector<float>& GetCoeffs() { return coeffs;}
	std::unordered_map<std::string, int>& GetBlendShapeNames() { return blendShapeMap;}
	std::vector<float>& GetHeadRotation() {return headRotation;}


	//载入表情驱动
	bool LoadInBlendShape(const std::string& file_path);

	//载入表情驱动
	bool LoadInBlendShapePre(const std::string& file_path);


private:
	void *m_AnalyserC;
	AnalyserResult *m_AnalyResult;
	std::vector<float> coeffs;
	std::vector<float> headRotation;
	std::vector<float> eyegazeDirectionVector;
	std::vector<CC3DFilter*> m_Filter;

	std::vector<CC3DEffectData>m_AllEffectData;
	std::vector<float>m_preScale;
	std::vector<float>m_postScale;

	TrackingResult mTr;


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
};

