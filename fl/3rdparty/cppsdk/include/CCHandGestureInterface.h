#ifndef CCHANDGESTUREINTERFACE_H
#define CCHANDGESTUREINTERFACE_H
#define CC_VERSION "0.4.0"
#include "HandGestureDef.h"
#define CC_MAX_HAND_COUNT 6
//const char* gesture_names[] =
//{ "1", "2", "3", "4", "5", "6", "8", "s-heart", "ok", "like", "dislike",
//  "fist", "rock", "palms_together","hold_fist", "salute","d-heart", "else" };
// 手包围框结构
typedef struct ccHGRect_t {
	float left;
	float top;
	float right;
	float bottom;
}ccHGRect;

// 每个关键点的坐标结构
typedef struct ccHGKeypoint_t {
	float x;
	float y;
}ccHGKeypoint;

// 每只手的信息结构
typedef struct ccHGHandInfo_t {
	ccHGRect handRect;	// 手包围框
	ccHGKeypoint keyPointArr[HANDKEYPOINTNUM]; // 手部21个关键点位置
	int handType; // 0右手，1左手
	int gestureType; // 手势识别类型
}ccHGHandInfo;


typedef struct ccHGHandRes_t {
	ccHGHandInfo handInfo[CC_MAX_HAND_COUNT]; // 每只手的信息
	int numHand; // 当前结果中手的数量
}ccHGHandRes;


HANDGESTURE_EXPORTS_API void ccHGSetMaxHandNum(int num); // 设置手检测数量上限
HANDGESTURE_EXPORTS_API bool ccHGInit(const char* model_path); // 用模型路径进行初始化
HANDGESTURE_EXPORTS_API void ccHGUnInit(); // 反初始化释放资源
HANDGESTURE_EXPORTS_API void ccHGReset(); // 重置，清空临时历史记录
HANDGESTURE_EXPORTS_API void ccHGSetGestureLooseness(float value);//手势分类松弛系数，越大越容易分为手势，0.0~10.0之间。
HANDGESTURE_EXPORTS_API void ccHGSetOnlyRightHand(bool only_right_hand); //是否只检测右手，可节省左手检测时间提高接口性能，有可能影响精度
HANDGESTURE_EXPORTS_API void ccHGSetDetFrequency(int value); //设置手检测器检测频率，每x帧检测一次
HANDGESTURE_EXPORTS_API void ccHGDetect(unsigned char* pBuffer, int width, int height, int rowBytes, int ctType); // 输入图片进行检测
HANDGESTURE_EXPORTS_API void ccHGGetHandRes(ccHGHandRes* res); // 获取预测结果
HANDGESTURE_EXPORTS_API void ccHGVersion(); // 打印当前sdk版本
#endif //#ifndef CCHANDGESTUREINTERFACE_H