#pragma once
#include "inc.h"
#include "BaseDefine/Vectors.h"
#include "ToolBox/CC3DQuaternion.h"


namespace CC3DImageFilter
{
	class CC3DSkeleton;
	struct BodyDriveBoneInfo
	{
		Vector3 Translate;
		Vector4 Quat;
		Vector4 InitQuat;
		Vector4 InitTranslate;
		std::string BoneName;
	};


	class CC3DBodyDriveConfig
	{
	public:
		CC3DBodyDriveConfig() = default;
		~CC3DBodyDriveConfig();

		void LoadConfig(const std::string& fileName, CC3DSkeleton* pSkeleton);
		const std::vector<std::vector<BodyDriveBoneInfo>>& GetBoneInfoList() const
		{
			return AnimationList;
		}
	private:
		std::map<std::string, std::string> BoneMaps{ {"f_avg_root","root"},{"f_avg_Pelvis","pelvis"},
			{"f_avg_L_Hip","thigh_l"},{"f_avg_R_Hip","thigh_r"},{"f_avg_Spine1","spine_01"},{"f_avg_L_Knee","calf_l"},
			{"f_avg_R_Knee","calf_r"},{"f_avg_Spine2","spine_02"},{"f_avg_L_Ankle","foot_l"},{"f_avg_R_Ankle","foot_r"},
			{"f_avg_Spine3","spine_03"},{"f_avg_L_Foot","ball_l"},{"f_avg_R_Foot","ball_r"},{"f_avg_Neck","neck_01"},
			{"f_avg_L_Collar","clavicle_l"},{"f_avg_R_Collar","clavicle_r"},{"f_avg_Head","head"},{"f_avg_L_Shoulder","upperarm_l"},
			{"f_avg_R_Shoulder","upperarm_r"},{"f_avg_L_Elbow","lowerarm_l"},{"f_avg_R_Elbow","lowerarm_r"},{"f_avg_L_Wrist","hand_l"},
			{"f_avg_R_Wrist","hand_r"},{"f_avg_lindex0","index_01_l"},{"f_avg_lindex1","index_02_l"},{"f_avg_lindex2","index_03_l"},
			{"f_avg_lmiddle0","middle_01_l"},{"f_avg_lmiddle1","middle_02_l"},{"f_avg_lmiddle2","middle_03_l"},{"f_avg_lpinky0","pinky_01_l"},
			{"f_avg_lpinky1","pinky_02_l"},{"f_avg_lpinky2","pinky_03_l"},{"f_avg_lring0","ring_01_l"},{"f_avg_lring1","ring_02_l"},
			{"f_avg_lring2","ring_03_l"},{"f_avg_lthumb0","thumb_01_l"},{"f_avg_lthumb1","thumb_02_l"},{"f_avg_lthumb2","thumb_03_l"},
			{"f_avg_rindex0","index_01_r"},{"f_avg_rindex1","index_02_r"},{"f_avg_rindex2","index_03_r"},{"f_avg_rmiddle0","middle_01_r"},
			{"f_avg_rmiddle1","middle_02_r"},{"f_avg_rmiddle2","middle_03_r"},{"f_avg_rpinky0","pinky_01_r"},{"f_avg_rpinky1","pinky_02_r"},
			{"f_avg_rpinky2","pinky_03_r"},{"f_avg_rring0","ring_01_r"},{"f_avg_rring1","ring_02_r"},{"f_avg_rring2","ring_03_r"},
			{"f_avg_rthumb0","thumb_01_r"},{"f_avg_rthumb1","thumb_02_r"},{"f_avg_rthumb2","thumb_03_r"} };
	
		std::vector<std::vector<BodyDriveBoneInfo>> AnimationList;
	};
}