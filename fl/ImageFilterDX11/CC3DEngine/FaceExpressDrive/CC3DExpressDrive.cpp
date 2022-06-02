#include "CC3DExpressDrive.h"
#include <iostream>
#include <map>

#include <fstream>
#include "Toolbox/json.hpp"

using namespace nlohmann;

float& get(TrackingResult& pTr, int pIndex)
{
	static int lBSCount = 58;
	if (pIndex >= lBSCount)
	{
		return pTr.headPose[pIndex - lBSCount + 3];
	}
	else
	{
		return pTr.au[pIndex].second;
	}
}

double curveMapping(CC3DCurveType pType, double pValue)
{
	auto x = pValue < 0 ? 0 : pValue > 1 ? 1 : pValue;
	switch (pType)
	{
	case CC3DCurveType::CC3DNORMAL:
		return x;
	case CC3DCurveType::CC3DEASE_IN_SINE:
		return 1 - cos((x * CC_PI) / 2);
	case CC3DCurveType::CC3DEASE_OUT_SINE:
		return sin((x * CC_PI) / 2);
	case CC3DCurveType::CC3DEASE_IN_OUT_SINE:
		return -(cos(CC_PI * x) - 1) / 2;
	case CC3DCurveType::CC3DEASE_IN_QUAD:
		return x * x;
	case CC3DCurveType::CC3DEASE_OUT_QUAD:
		return 1 - (1 - x) * (1 - x);
	case CC3DCurveType::CC3DEASE_IN_OUT_QUAD:
		return x < 0.5 ? 2 * x * x : 1 - pow(-2 * x + 2, 2) / 2;
	case CC3DCurveType::CC3DEASE_IN_CUBIC:
		return x * x * x;
	case CC3DCurveType::CC3DEASE_OUT_CUBIC:
		return 1 - pow(1 - x, 3);
	case CC3DCurveType::CC3DEASE_IN_OUT_CUBIC:
		return x < 0.5 ? 4 * x * x * x : 1 - pow(-2 * x + 2, 3) / 2;
	case CC3DCurveType::CC3DEASE_IN_QUART:
		return x * x * x * x;
	case CC3DCurveType::CC3DEASE_OUT_QUART:
		return 1 - pow(1 - x, 4);
	case CC3DCurveType::CC3DEASE_IN_OUT_QUART:
		return x < 0.5 ? 8 * x * x * x * x : 1 - pow(-2 * x + 2, 4) / 2;
	case CC3DCurveType::CC3DEASE_IN_QUINT:
		return x * x * x * x * x;
	case CC3DCurveType::CC3DEASE_OUT_QUINT:
		return 1 - pow(1 - x, 5);
	case CC3DCurveType::CC3DEASE_IN_OUT_QUINT:
		return x < 0.5 ? 16 * x * x * x * x * x : 1 - pow(-2 * x + 2, 5) / 2;
	case CC3DCurveType::CC3DEASE_IN_EXPO:
		// ===
		return x == 0 ? 0 : pow(2, 10 * x - 10);
	case CC3DCurveType::CC3DEASE_OUT_EXPO:
		// ===
		return x == 1 ? 1 : 1 - pow(2, -10 * x);
	case CC3DCurveType::CC3DEASE_IN_OUT_EXPO:
		// ===
		return x == 0
			? 0
			// ===
			: x == 1
			? 1
			: x < 0.5 ? pow(2, 20 * x - 10) / 2
			: (2 - pow(2, -20 * x + 10)) / 2;
	case CC3DCurveType::CC3DEASE_IN_CIRC:
		return 1 - sqrt(1 - pow(x, 2));
	case CC3DCurveType::CC3DEASE_OUT_CIRC:
		return sqrt(1 - pow(x - 1, 2));
	case CC3DCurveType::CC3DEASE_IN_OUT_CIRC:
		return x < 0.5
			? (1 - sqrt(1 - pow(2 * x, 2))) / 2
			: (sqrt(1 - pow(-2 * x + 2, 2)) + 1) / 2;
	case CC3DCurveType::CC3DEASE_IN_BACK:
	{
		double c1 = 1.70158;
		double c3 = c1 + 1;

		return c3 * x * x * x - c1 * x * x;
	}
	case CC3DCurveType::CC3DEASE_OUT_BACK:
	{
		double c1 = 1.70158;
		double c3 = c1 + 1;

		return 1 + c3 * pow(x - 1, 3) + c1 * pow(x - 1, 2);
	}
	case CC3DCurveType::CC3DEASE_IN_OUT_BACK:
	{
		double c1 = 1.70158;
		double c2 = c1 + 1;

		return x < 0.5
			? (pow(2 * x, 2) * ((c2 + 1) * 2 * x - c2)) / 2
			: (pow(2 * x - 2, 2) * ((c2 + 1) * (x * 2 - 2) + c2) + 2) / 2;
	}
	case CC3DCurveType::CC3DEASE_IN_ELASTIC:
	{
		double c4 = (2 * CC_PI) / 3;

		// ===
		return x == 0
			? 0
			// ===
			: x == 1
			? 1
			: -pow(2, 10 * x - 10) * sin((x * 10 - 10.75) * c4);
	}
	case CC3DCurveType::CC3DEASE_OUT_ELASTIC:
	{
		double c4 = (2 * CC_PI) / 3;

		// ===
		return x == 0
			? 0
			// ===
			: x == 1
			? 1
			: pow(2, -10 * x) * sin((x * 10 - 0.75) * c4) + 1;
	}
	case CC3DCurveType::CC3DEASE_IN_OUT_ELASTIC:
	{
		double c5 = (2 * CC_PI) / 4.5;

		// ===
		return x == 0
			? 0
			// ===
			: x == 1
			? 1
			: x < 0.5
			? -(pow(2, 20 * x - 10) * sin((20 * x - 11.125) * c5)) / 2
			: (pow(2, -20 * x + 10) * sin((20 * x - 11.125) * c5)) / 2 + 1;
	}
	case CC3DCurveType::CC3DEASE_IN_BOUNCE:
		return 1 - curveMapping(CC3DCurveType::CC3DEASE_OUT_BOUNCE, 1 - x);
	case CC3DCurveType::CC3DEASE_OUT_BOUNCE:
	{
		double n1 = 7.5625;
		double d1 = 2.75;

		if (x < 1 / d1) {
			return n1 * x * x;
		}
		else if (x < 2 / d1) {
			return n1 * (x -= 1.5 / d1) * x + 0.75;
		}
		else if (x < 2.5 / d1) {
			return n1 * (x -= 2.25 / d1) * x + 0.9375;
		}
		else {
			return n1 * (x -= 2.625 / d1) * x + 0.984375;
		}
	}
	case CC3DCurveType::CC3DEASE_IN_OUT_BOUNCE:
		return x < 0.5
			? (1 - curveMapping(CC3DCurveType::CC3DEASE_OUT_BOUNCE, 1 - 2 * x)) / 2
			: (1 + curveMapping(CC3DCurveType::CC3DEASE_OUT_BOUNCE, 2 * x - 1)) / 2;
	default:
		return x;
	}
}


std::map<std::string, CC3DModifyType> ModifyDict
{
	{"禁用", CC3DModifyType::CC3DEFFECT},
	{"截断", CC3DModifyType::CC3DEFFECT},
	{"默认值", CC3DModifyType::CC3DEFFECT},
	{"曲线映射", CC3DModifyType::CC3DEFFECT},
	{"表情联动（加强）", CC3DModifyType::CC3DEFFECT},
	{"表情联动（减弱）", CC3DModifyType::CC3DEFFECT},
	{"强制对称", CC3DModifyType::CC3DEFFECT},
	{"均值滤波", CC3DModifyType::CC3DFILTER},
	{"高斯滤波", CC3DModifyType::CC3DFILTER},
	{"双边滤波", CC3DModifyType::CC3DFILTER}
};


std::map<std::string, CC3DEffectType> EffectDict{
	{"禁用", CC3DEffectType::CC3DDEACTIVE},
	{"截断", CC3DEffectType::CC3DCLAMP},
	{"默认值", CC3DEffectType::CC3DOFFSET},
	{"曲线映射", CC3DEffectType::CC3DCURVE},
	{"表情联动（加强）", CC3DEffectType::CC3DINCREASE_BY},
	{"表情联动（减弱）",CC3DEffectType::CC3DREDUCE_BY},
	{"强制对称", CC3DEffectType::CC3DMIRROR}
};

std::map<std::string, CC3DFilterType> FilterDict{
	{"均值滤波", CC3DFilterType::CC3DMEAN},
	{"高斯滤波", CC3DFilterType::CC3DGAUSS},
	{"双边滤波", CC3DFilterType::CC3DBILATERAL}
};
void CC3DFilter::process(TrackingResult& pTr)
{
	static int lBSCount = 58;

	if (!mEffectData->mIsActive) return;

	auto  index = mEffectData->mIndices[0];

	double pValue = pTr.au[index].second;
	if (index >= lBSCount)
	{
		pValue = pTr.headPose[index - lBSCount + 3];
	}

	mValues[mStart] = pValue;

	double lRes = pValue;

	switch (FilterDict[mEffectData->mEffectName])
	{
	case CC3DFilterType::CC3DMEAN:
	{
		double sum = 0;
		for (int i = 0; i < mLength; ++i)
		{
			sum += mValues[i];
		}
		lRes = sum / mLength;
		break;
	}
	case CC3DFilterType::CC3DGAUSS:
	{
		double lValue = 0;
		double lWeight = 0;
		for (int i = 0; i < mLength; ++i)
		{
			auto lIndex = i + mStart + 1;
			if (lIndex >= mLength)
			{
				lIndex -= mLength;
			}
			lValue += mValues[lIndex] * mPrecpt[i];
			lWeight += mPrecpt[i];
		}
		lRes = lValue / lWeight;
		break;
	}
	case CC3DFilterType::CC3DBILATERAL:
	{
		double lMin = *std::min_element(mValues.cbegin(), mValues.cend());
		double lMax = *std::max_element(mValues.cbegin(), mValues.cend());

		if (lMax - lMin < 0.001)
		{
			break;
		}

		double lCoeffValue = -0.5 / (mEffectData->mParams[2] * mEffectData->mParams[2]);
		double lValue = 0;
		double lWeight = 0;

		for (int i = 0; i < mLength; ++i)
		{
			auto lIndex = i + mStart + 1;
			if (lIndex >= mLength)
			{
				lIndex -= mLength;
			}
			double lCenterValue = mValues[mStart];
			double lVal = mValues[lIndex];
			double lDelta = abs(lVal - lCenterValue);

			double lWeightValue = exp(lDelta * lDelta * lCoeffValue);
			double lWeightCombine = mPrecpt[i] * lWeightValue;

			lValue += lVal * lWeightCombine;
			lWeight += lWeightCombine;
		}
		lRes = lValue / lWeight;
		break;
	}
	default:
		lRes = pValue;
		break;
	}

	++mStart;
	if (mStart >= mLength)
	{
		mStart -= mLength;
	}

	if (index >= lBSCount)
	{
		pTr.headPose[index - lBSCount + 3] = lRes;
	}
	else
	{
		pTr.au[index].second = lRes;
	}
}

void effect_process(TrackingResult& pTr, CC3DEffectData& pEffectData)
{
	if (!pEffectData.mIsActive) return;

	switch (EffectDict[pEffectData.mEffectName])
	{
	case CC3DEffectType::CC3DDEACTIVE:
	{
		// index
		auto lIndex = pEffectData.mIndices[0];
		//pTr.au[lIndex].second = 0;
		get(pTr, lIndex) = 0;
		break;
	}
	case CC3DEffectType::CC3DCLAMP:
	{
		// index, min, max
		auto lIndex = pEffectData.mIndices[0];
		auto& lValue = get(pTr, lIndex);
		if (lValue < pEffectData.mParams[0])
		{
			lValue = pEffectData.mParams[0];
		}
		else if (lValue > pEffectData.mParams[1])
		{
			lValue = pEffectData.mParams[1];
		}
		//pTr.au[lIndex].second = lValue;
		break;
	}
	case CC3DEffectType::CC3DOFFSET:
	{
		// index, offset
		auto lIndex = pEffectData.mIndices[0];
		get(pTr, lIndex) += pEffectData.mParams[0];
		break;
	}
	case CC3DEffectType::CC3DCURVE:
	{
		// index, curve index
		auto lIndex = pEffectData.mIndices[0];
		auto& lValue = get(pTr, lIndex);
		lValue = curveMapping(CC3DCurveType(int(pEffectData.mParams[0])), lValue);
		break;
	}
	case CC3DEffectType::CC3DINCREASE_BY:
	{
		// target index, source index, curve index, scale
		auto lTarIndex = pEffectData.mIndices[0];
		auto lSrcIndex = pEffectData.mIndices[1];
		get(pTr, lTarIndex) +=
			curveMapping(CC3DCurveType(int(pEffectData.mParams[0])), get(pTr, lSrcIndex)) * pEffectData.mParams[1];
		break;
	}
	case CC3DEffectType::CC3DREDUCE_BY:
	{
		// target index, source index, curve index, scale
		auto lTarIndex = pEffectData.mIndices[0];
		auto lSrcIndex = pEffectData.mIndices[1];
		get(pTr, lTarIndex) -=
			curveMapping(CC3DCurveType(int(pEffectData.mParams[0])), get(pTr, lSrcIndex)) * pEffectData.mParams[1];
		break;
	}
	case CC3DEffectType::CC3DMIRROR:
	{
		// index1, index2, weight1, weight2
		auto lIndex0 = pEffectData.mIndices[0];
		auto lIndex1 = pEffectData.mIndices[1];
		auto& lValue0 = get(pTr, lIndex0);
		auto& lValue1 = get(pTr, lIndex1);
		auto lTargetValue = lValue0 * pEffectData.mParams[0] + lValue1 * pEffectData.mParams[1];
		lValue0 = lTargetValue;
		lValue1 = lTargetValue;
		break;
	}
	default:
		break;
	}
}
CC3DExpressDrive::CC3DExpressDrive()
{
	m_AnalyserC = NULL;
	m_AnalyResult = NULL;
	for (int i=0;i<m_BlendShapeName2.size();i++)
	{
		blendShapeMap[m_BlendShapeName2[i]] = i;
		m_BlendShapeName2[i][0] = m_BlendShapeName[i][0] + 'A' - 'a';
		blendShapeMap[m_BlendShapeName2[i]] = i;
	}
	for (int i = 0; i < m_BlendShapeName.size(); i++)
	{
		blendShapeMap[m_BlendShapeName[i]] = i;
	}

}

CC3DExpressDrive::~CC3DExpressDrive()
{
#ifndef _WIN64
	analyserRelease(m_AnalyserC);
	SAFE_DELETE(m_AnalyResult);

	for (int i = 0; i < m_Filter.size(); i++)
	{
		SAFE_DELETE(m_Filter[i]);
	}
#endif
}

void CC3DExpressDrive::SetModelPath(const char* modelPath)
{
#ifndef _WIN64
	if (m_AnalyserC == NULL)
	{
		m_AnalyserC = analyserInitPath(modelPath);
	}
	if (m_AnalyResult == NULL)
	{
		m_AnalyResult = new AnalyserResult();
	}
#endif
	//setAnalyserTongueFusionState(m_AnalyserC, false);

// 	LoadInBlendShapePre("../Release/Resources/editor_intro.cf");
// 	LoadInBlendShape("../Release/Resources/live_mouthPucker.fx");
	
}

void CC3DExpressDrive::FacePointIndexTransfer(Vector2 * pSrcPoint130, std::vector<float>& mDstPoint74)
{
	Vector2 nDstPoint74[74];
	for (int i = 0; i < 9; i++)
	{
		nDstPoint74[i] = pSrcPoint130[i * 2];
	}
	for (int j = 9; j < 17; j++)
	{
		int index = 50 - 2 * j;
		nDstPoint74[j] = pSrcPoint130[index];
	}
	for (int j = 17; j < 22; j++)
	{
		int index = j + 16;
		nDstPoint74[j] = pSrcPoint130[index];
	}
	for (int j = 22; j < 27; j++)
	{
		int index = j + 20;
		nDstPoint74[j] = pSrcPoint130[index];
	}
	for (int j = 22; j < 27; j++)
	{
		int index = j + 20;
		nDstPoint74[j] = pSrcPoint130[index];
	}
	for (int j = 27; j < 31; j++)
	{
		int index = j + 44;
		nDstPoint74[j] = pSrcPoint130[index];
	}
	for (int j = 31; j < 36; j++)
	{	
		int index = j + 47;
		if (j == 32 || j == 34)
		{
			nDstPoint74[j] = (pSrcPoint130[index - 1]+ pSrcPoint130[index + 1]) *0.5;
		}
		else 
		{
			nDstPoint74[j] = pSrcPoint130[index];
		}	
	}
	for (int j = 36; j < 45; j++)
	{
		int index = j + 15;
		nDstPoint74[j] = pSrcPoint130[index];
	}
	for (int j = 45; j < 54; j++)
	{
		int index = j + 16;
		nDstPoint74[j] = pSrcPoint130[index];
	}
	nDstPoint74[54] = pSrcPoint130[86];
	nDstPoint74[55] = pSrcPoint130[88];
	nDstPoint74[56] = pSrcPoint130[87];
	nDstPoint74[57] = pSrcPoint130[98];
	nDstPoint74[58] = pSrcPoint130[99] ;

	nDstPoint74[59] = pSrcPoint130[92];
	nDstPoint74[60] = pSrcPoint130[90];
	nDstPoint74[61] = pSrcPoint130[91];
	nDstPoint74[62] = pSrcPoint130[102];
	nDstPoint74[63] = pSrcPoint130[101];
	
	nDstPoint74[64] = pSrcPoint130[89];
	nDstPoint74[65] = pSrcPoint130[100];

	nDstPoint74[66] = pSrcPoint130[103];
	nDstPoint74[67] = pSrcPoint130[93];
	nDstPoint74[68] = pSrcPoint130[94];
	
	nDstPoint74[69] = pSrcPoint130[105];
	nDstPoint74[70] = pSrcPoint130[97];
	nDstPoint74[71] = pSrcPoint130[96];

	nDstPoint74[72] = pSrcPoint130[104];
	nDstPoint74[73] = pSrcPoint130[95];

	std::vector<float> nPoint(148);
	for (int k = 0; k < 74; k++)
	{
		nPoint[k] = nDstPoint74[k].x;
		nPoint[k + 74] = nDstPoint74[k].y;
	}
	mDstPoint74 = nPoint;
}

void CC3DExpressDrive::PostProcess()
{
	for (int i=0;i<m_preScale.size();i++)
	{
		mTr.au[i].second *= m_preScale[i];
	}
	for (int i = 0; i < m_postScale.size(); i++)
	{
		mTr.au[i].second *= m_postScale[i];
	}

	for (int i = 0;i<m_AllEffectData.size();i++)
	{
		if (ModifyDict[m_AllEffectData[i].mEffectName] == CC3DModifyType::CC3DEFFECT)
		{
			effect_process(mTr, m_AllEffectData[i]);
		}
	}

	if (m_Filter.size() == 0)
	{
		for (int i = 0; i < m_AllEffectData.size(); i++)
		{
			if (ModifyDict[m_AllEffectData[i].mEffectName] == CC3DModifyType::CC3DFILTER)
			{
				auto filterType = FilterDict[m_AllEffectData[i].mEffectName];
				CC3DFilter *pFilter = new CC3DFilter(filterType, m_AllEffectData[i]);
				m_Filter.push_back(pFilter);
			}
		}

	}

	for (int i=0;i<m_Filter.size();i++)
	{
		m_Filter[i]->process(mTr);
	}

	for (int i = 0; i < coeffs.size(); ++i)
	{
		//mTr.au[i].first = i + 1;
		coeffs[i] = mTr.au[i].second;
	}
}

void CC3DExpressDrive::GetBlendshapeWeights(unsigned char *data, int width, int height,  Vector2 * faceRes)
{
#ifndef _WIN64
	if (faceRes != NULL)
	{
		Vector2* pPoint130 = faceRes;
		
		std::vector<float> mDstPoint74;
		FacePointIndexTransfer(pPoint130, mDstPoint74);

		if (!mDstPoint74.empty())
		{			
			analyserUpdate(m_AnalyserC, height, width, data, mDstPoint74.data(), 148);
			getAnalyserResult(m_AnalyserC, m_AnalyResult);
			if (coeffs.size() != 52)
			{
				coeffs.resize(52);
			}
			memcpy(&coeffs[0], m_AnalyResult->expressions, 52 * sizeof(float));
			
			if (headRotation.size() != 3)
			{
				headRotation.resize(3);
			}
			memcpy(&headRotation[0], m_AnalyResult->rotation, 3 * sizeof(float));

			// Transfer eye gaze to blendshape weights
			std::vector<float> degree(2);
			memcpy(&degree[0], m_AnalyResult->gazeDegree, 2 * sizeof(float));
			//std::cout << degree[0] << " " << degree[1] << std::endl;
			// 左 degree[1] [0,-0.16  右 degree[1] [0,0.16] 上 degree[0] [0,-0.08] 下 degree[0] [0,0.08]
			float up = degree[0] < 0 ? abs(degree[0] / 3.14f * 20) : 0;
			float down = degree[0] > 0 ? degree[0] / 3.14f * 20 : 0;
			float left = degree[1] < 0 ? abs(degree[1] / 3.14f * 15) : 0;
			float right = degree[1] > 0 ? degree[1] / 3.14f * 15 : 0;

			coeffs[37] = down;
			coeffs[38] = down;
			coeffs[43] = up;
			coeffs[44] = up;

			coeffs[40] = left;
			coeffs[41] = left;
			coeffs[39] = right;
			coeffs[42] = right;

			for (int i = 0; i < coeffs.size(); ++i)
			{
				mTr.au[i].first = i + 1;
				mTr.au[i].second = coeffs[i];
			}

			mTr.headPose[0] = 0.0f;
			mTr.headPose[1] = 0.0f;
			mTr.headPose[2] = 0.0f;
			mTr.headPose[3] = headRotation[0];
			mTr.headPose[4] = headRotation[1];
			mTr.headPose[5] = headRotation[2];
			PostProcess();
		}
	}
#endif
}


bool CC3DExpressDrive::LoadInBlendShape(const std::string& file_path)
{
	json root;
	std::ifstream ifile(file_path);
	ifile >> root;
	m_postScale.clear();
	auto Scales = root["Scales"];
	if (!Scales.is_null())
	{
		for (int ni = 0; ni < Scales.size(); ni++)
		{
			m_postScale.push_back(Scales[ni].get <float>());
		}
	}
	m_AllEffectData.clear();
	auto Effects = root["Effects"];
	if (!Effects.is_null())
	{
		m_AllEffectData.resize(Effects.size());
		for (int ni = 0; ni < Effects.size(); ni++)
		{
			json pf_json = Effects[ni];

			m_AllEffectData[ni].mEffectName = pf_json["EffectName"].get<std::string>();
			m_AllEffectData[ni].mIsActive = pf_json["IsActive"].get<bool>();

			auto nIndex = pf_json["Indices"];
			for (int i = 0; i < nIndex.size(); i++)
			{
				m_AllEffectData[ni].mIndices.push_back(nIndex[i].get <int>());
			}

			auto Params = pf_json["Params"];
			for (int i = 0; i < Params.size(); i++)
			{
				m_AllEffectData[ni].mParams.push_back(Params[i].get <float>());
			}
		}
	}

	return true;
}

bool CC3DExpressDrive::LoadInBlendShapePre(const std::string & file_path)
{
	json root;
	std::ifstream ifile(file_path);
	ifile >> root;

	m_preScale.clear();
	auto filters = root["Filters"];
	if (!filters.is_null())
	{
		for (int ni = 0; ni < filters.size(); ni++)
		{
			json pf_json = filters[ni][0];
			float param;
			auto Params = pf_json["Params"];
			param = Params[2].get<double>();
			m_preScale.push_back(param);
		}
	}
	return true;
}
