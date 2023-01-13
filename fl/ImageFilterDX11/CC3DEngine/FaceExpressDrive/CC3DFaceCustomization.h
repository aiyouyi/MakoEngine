#ifndef _CC3D_FACE_CUSTOMIZATION_H_
#define _CC3D_FACE_CUSTOMIZATION_H_

#include "Toolbox/inc.h"

class CC3DFaceCustomization
{
public:
	CC3DFaceCustomization();
	~CC3DFaceCustomization();

	void UpdateCoeffByBSName(const std::string& _name, float ratio);
	void ResetBlendShape();

	std::unordered_map<std::string, int>& GetFaceBlendShapeNames() { return blendShapeFllowIndex; }
	std::vector<float>& GetFaceCoeffs() { return _coeffs_array; }

	std::unordered_map<std::string, std::vector<std::string>> blendShapeNameMap = {
	{"chin", {"chin_up", "chin_dn", "chin_lf", "chin_rt"}},
	{ {"cheek"}, {"cheek_up", "cheek_dn", "cheek_lf", "cheek_rt"} },
	{ {"jaws"}, {"jaws_up", "jaws_dn", "jaws_lf", "jaws_rt"} },
	{ {"Eyebrows_rotation"}, {"Eyebrows_rotation_up", "Eyebrows_rotation_dn"} },
	{ {"Eyebrows"}, {"Eyebrows_up", "Eyebrows_dn", "Eyebrows_lf", "Eyebrows_rt"} },
	{ {"Eyebrows_mid"}, {"Eyebrows_mid_up", "Eyebrows_mid_dn", "Eyebrows_mid_lf", "Eyebrows_mid_rt"} },
	{ {"Eye_mobile"}, {"eye_mobile_up", "eye_mobile_dn", "eye_mobile_lf", "eye_mobile_rt"} },
	{ {"Eye_BS"}, {"narrow_best", "amplifiy_best"} },
	{ {"Upper_eyelid"}, {"Upper_eyelid_up", "Upper_eyelid_dn", "Upper_eyelid_lf", "Upper_eyelid_rt"} },
	{ {"lower_eyelid"}, {"lower_eyelid_up", "lower_eyelid_dn", "lower_eyelid_lf", "lower_eyelid_rt"} },
	{ {"Outside_canthus"}, {"Outside_canthus_up", "Outside_canthus_dn", "Outside_canthus_lf", "Outside_canthus_rt"} },
	{ {"inside_canthus"}, {"inside_canthus_up", "inside_canthus_dn", "inside_canthus_lf", "inside_canthus_rt"} },
	{ {"pupil"}, {"Dilated_L", "miosis_L"} },
	{ {"nose"}, {"nose_up", "nose_dn"} },
	{ {"biliang"}, {"nose_biliang_up", "nose_biliang_dn"} },
	{ {"biyi"}, {"nose_biyi_up", "nose_biyi_dn", "nose_biyi_lf", "nose_biyi_rt"} },
	{ {"bitou"}, {"nose_bitou_up", "nose_bitou_dn", "nose_bitou_lf", "nose_bitou_rt"} },
	{ {"lips"}, {"lips_up", "lips_dn", "lips_lf", "lips_rt"} },
	{ {"mouth"}, {"mouth_up", "mouth_dn", "mouth_lf", "mouth_rt"} },
	{ {"Upper_lip"}, {"Upper_lip_amplifiy", "Upper_lip_narrow"} },
	{ {"lower_lip"}, {"lower_lip_amplifiy", "lower_lip_narrow"} },
	{ {"up_Lipbead"}, {"up_Lipbead_up", "up_Lipbead_dn"} },
	{ {"dn_Lipbead"}, {"dn_Lipbead_up", "dn_Lipbead_dn"} },
	{ {"mid_Lipbead"}, {"mid_Lipbead_up", "mid_Lipbead_dn"} } };

private:
	std::vector<float> _coeffs_array;

	std::vector<std::string> _blendShapeName = {
		"chin_up", "chin_dn", "chin_lf", "chin_rt", //下巴
		"cheek_up", "cheek_dn", "cheek_lf", "cheek_rt", //脸颊
		"jaws_up", "jaws_dn", "jaws_lf", "jaws_rt",  //下颚骨
		"Eyebrows_rotation_up", "Eyebrows_rotation_dn", //眉毛旋转
		"Eyebrows_up", "Eyebrows_dn", "Eyebrows_lf", "Eyebrows_rt", //眉毛
		"Eyebrows_mid_up", "Eyebrows_mid_dn", "Eyebrows_mid_lf", "Eyebrows_mid_rt",//眉中
		"eye_mobile_up", "eye_mobile_dn", "eye_mobile_lf", "eye_mobile_rt",//眼睛位置
		"narrow_best", "amplifiy_best", //眼睛大小
		"Upper_eyelid_up", "Upper_eyelid_dn", "Upper_eyelid_lf", "Upper_eyelid_rt", //上眼皮
		"lower_eyelid_up", "lower_eyelid_dn", "lower_eyelid_lf", "lower_eyelid_rt", //下眼皮
		"Outside_canthus_up", "Outside_canthus_dn", "Outside_canthus_lf", "Outside_canthus_rt", //外眼角
		"inside_canthus_up", "inside_canthus_dn", "inside_canthus_lf", "inside_canthus_rt", //内眼角
		"Dilated_L", "miosis_L", //瞳孔
		"nose_up", "nose_dn", //鼻子长度
		"nose_biliang_up", "nose_biliang_dn", //鼻梁
		"nose_biyi_up", "nose_biyi_dn", "nose_biyi_lf", "nose_biyi_rt", //鼻翼
		"nose_bitou_up", "nose_bitou_dn", "nose_bitou_lf", "nose_bitou_rt", //鼻头
		"lips_up", "lips_dn", "lips_lf", "lips_rt", //嘴唇
		"mouth_up", "mouth_dn", "mouth_lf", "mouth_rt", //嘴巴
		"Upper_lip_amplifiy", "Upper_lip_narrow", //上唇
		"lower_lip_amplifiy", "lower_lip_narrow", //下唇
		"up_Lipbead_up", "up_Lipbead_dn", //上唇珠
		"dn_Lipbead_up", "dn_Lipbead_dn", //下唇珠
		"mid_Lipbead_up", "mid_Lipbead_dn", //中唇珠

		"Ankle","Dcup"                  ,//胸部
		//跟随的blendshape
	};


	std::unordered_map<std::string, std::vector<std::string>> blendShapeMap = {
	{ {"miosis_L"}, {"miosis_R"} },
	{ {"Dilated_L"}, {"Dilated_R","Dilated_eye_r","Dilated_eye_l"} }
	};

	std::unordered_map<std::string, int> blendShapeFllowIndex;
};
#endif