#include "CC3DBodyDriveConfig.h"
#include "json.hpp"
#include "Skeleton/CC3DSkeleton.h"

namespace CC3DImageFilter
{
	CC3DBodyDriveConfig::~CC3DBodyDriveConfig()
	{

	}

	void CC3DBodyDriveConfig::LoadConfig(const std::string& fileName, CC3DSkeleton* pSkeleton)
	{
		AnimationList.clear();

		std::ifstream input_json_file(fileName);
		nlohmann::json AnimateJson;
		input_json_file >> AnimateJson;
		if (AnimateJson.is_array())
		{
			for (auto it = AnimateJson.begin(); it != AnimateJson.end(); ++it)
			{
				const nlohmann::json& Obj = *it;
				if (Obj.find("data") == Obj.end())
				{
					return;
				}
				const nlohmann::json& Data = Obj["data"];
				if (Data.find("keyquaternions") == Data.end())
				{
					return;
				}
				const nlohmann::json& KeyQuaternions = Data["keyquaternions"];
				if (KeyQuaternions.is_array())
				{
					std::vector< BodyDriveBoneInfo> FrameList;
					for (auto QuatIt = KeyQuaternions.begin(); QuatIt != KeyQuaternions.end(); ++QuatIt)
					{
						BodyDriveBoneInfo BoneInfo;
						std::string BoneName = (*QuatIt)["name"];
						
						BoneInfo.BoneName = BoneName;

						const nlohmann::json& QuatObj = (*QuatIt)["quaternion"];

						BoneInfo.Quat.w = QuatObj["w"];
						BoneInfo.Quat.x =  QuatObj["x"];
						BoneInfo.Quat.y =  QuatObj["y"];
						BoneInfo.Quat.z = QuatObj["z"];


						float yaw, pitch, roll;
						CC3DMath::CC3DQuaternion TempQuat(&BoneInfo.Quat.x);
						TempQuat.computeEuler(&yaw, &pitch, &roll);


						CC3DMath::CC3DQuaternion::createFromEuler(yaw, pitch, roll, &TempQuat);

						BoneInfo.Quat.w = TempQuat.w;
						BoneInfo.Quat.x = TempQuat.x;
						BoneInfo.Quat.y = TempQuat.y;
						BoneInfo.Quat.z = TempQuat.z;

						
						const nlohmann::json& TranslationObj = (*QuatIt)["translation"];

						BoneInfo.Translate.x = TranslationObj["x"];
						BoneInfo.Translate.y = TranslationObj["y"];
						BoneInfo.Translate.z = TranslationObj["z"];

						int32_t NodeId = -1;
						if (pSkeleton->m_BoneMap.find(BoneInfo.BoneName) == pSkeleton->m_BoneMap.end())
						{
							continue;
						}
						NodeId = pSkeleton->m_BoneMap.find(BoneInfo.BoneName)->second;
						CC3DBoneNodeInfo* pBoneInfo = pSkeleton->m_BoneNode[NodeId];
						BoneInfo.InitQuat = pBoneInfo->Rotation;
						BoneInfo.Translate = pBoneInfo->Translate;

						FrameList.push_back(BoneInfo);
					}
					AnimationList.push_back(FrameList);
				}
			}
		}
	}
}