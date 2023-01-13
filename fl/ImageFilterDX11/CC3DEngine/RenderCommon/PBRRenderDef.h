#pragma once
#include "inc.h"
#include "BaseDefine/Vectors.h"

namespace CC3DImageFilter
{
	struct SkinInfo
	{
		float SkinSmooth = 0.65f;
		float SkinSpecularScale = 0.6f;
		float SkinCurveFactor = 1.0;
		Vector3 SurfaceColor{ 0.9,0.7,0.83 };
	};


	struct MeshDisInfo
	{
		float Distance;
		int MeshID;
		int ModelID;
		//区分mesh包围框的最近最远点
		int PosType;

		bool operator()(const MeshDisInfo& pNear, const MeshDisInfo& pFar)
		{
			return pNear.Distance > pFar.Distance;
		}
	};


	struct KajiyaHairInfoConfig
	{
		float SecondaryShift = 0.2;
		float PrimaryShift = 0.2;
		float SpecularPower = 50.0;
		float ShiftU = 1.0;
		float SpecularScale = 0.6;
		float SpecularWidth = 0.5;
		float SpecularStrength = 1.0;
		Vector4 KajiyaSpecularColor = { 1.0,1.0,1.0,1.0 };
	};

	struct MakeupParam
	{
		std::string makeup_name; //模型或者图片的文件名带后缀
		unsigned int id;
		std::string model_type;
		Vector2 rect[4];
		std::string texture_name;
		std::string model_name;
		std::string attach_name;
		std::string absolute_path; //选择模型或者图片的绝对路径
		std::unordered_map<std::string, Vector3> colorChangeMap;
		KajiyaHairInfoConfig Kajiya;
	};

	struct EmissMapParam
	{
		std::string mtl_name;
		float stength = 0.0f;
		float radius = 1.0f;
	};

	struct PBRNoramlInfo
	{
		float NormalIntensity = 0.0;
		float FrontNormalOffset = 0.0;
		float FrontNormalScale = 1.0f;
	};

}