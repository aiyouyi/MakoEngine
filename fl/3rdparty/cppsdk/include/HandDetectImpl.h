#ifndef HAND_DETECT_IMPL_H
#define HAND_DETECT_IMPL_H

#include <string>
#include <list>
#include <cmath>

#include "opencv2/opencv.hpp"
#include "HandDetectDef.h"
#include "HandPoseEstimation/HandPoseModel.h"
// #include "HandPoseEstimation/ImageHelper.h"
#include "HandPoseEstimation/Filter.h"
#include "HandDetect/HandDetector.h"
#include "SingleHandGesture/SingleGestureModel.h"
#include "DoubleHandGesture/DoubleGestureModel.h"
// #include "Utils/utils.h"
#include "HandPoseEstimation/mathlib.h"

using namespace std;

class HandInfo {
public:
	HandInfo();
public:
	float keypoints[HAND_KEYPOINT_NUM * 3] = { 0. };
	int handType;		// -1: unknown 0: right 1: left
	float handTypeProb;
	int gestureType;
	int clsErrorCount;
	int doubleGestureID;
	vec2 movDirection;
	cv::Rect handBox;
};

class HandDetectImpl {
public:
	HandDetectImpl();
	~HandDetectImpl();

	//------------------------------------ Detect接口 ------------------------------------//
	bool init(const char* szModelPath, int deviceMode);
	void uninit();
	void reset();
	bool detect(const unsigned char* pBuffer, int width, int height, int stride, int ctType);
	ccHandRes getResult();
	ccGestureRes getGestureResult();

	//------------------------------------ ROI接口 ------------------------------------//
	void setMaxHandNum(int numHand);
	void setRoiDetectSkipFrames(int numSkipFrames);

	//------------------------------------ Pose接口 ------------------------------------//
	bool activateHandPose();
	void deactivateHandPose();
	void setPoseSmoothWeight(float value);
	void setOnlyRightHand(bool value);

	//------------------------------------ Gesture接口 ------------------------------------//
	// 单手手势
	bool activateSingleGesture();
	void deactivateSingleGesture();
	void setSingleGestureRelaxation(float value);

	// 双手手势
	bool activateDoubleGesture();
	void deactivateDoubleGesture();
	void setDoubleGestureRelaxation(float value);
	

private:
	bool initPoseModel();
	
	string getModelDir(const char* pModelDir);
	cv::Mat getMatImg(const unsigned char* pBuffer, int width, int height, int stride, int ctType);
	cv::Rect extendBBox(cv::Rect r, const float* keypoints);
	cv::Rect keypoints2BBox(const float* keypoints);

	void predKeypoints(cv::Mat& frame, int width, int height, int ctType);
	//void setOnlyRightHand(bool value);
	int getNumOfHandDetected();
	void updateHandList(const cv::Mat& img, int width, int height, int ctType);
	void predDoubleGesture();
	float calcHandSimilarity(float* hand1, float* hand2);
	float calcHandDistance(float* hand1, float* hand2);
	void removeErrorHand();
private:
	// 最小可检测手面积
	static const int m_cMinHandSize = 1000;

	// 输入图像最短边限制
	const int m_cMinImgEdge = 50;

	// 检测器
	HandDetector* m_pHandDetector;
	HandPoseModel* m_pHandposeModel;
	SingleGestureModel* m_pSingleGestureModel;
	DoubleGestureModel* m_pDoubleGestureModel;

	// 检测结果
	ccHandRes m_handRes;
	ccGestureRes m_gestureRes;

	// 模块开关
	bool m_bUsePose;
	bool m_bUseSingleGesture;
	bool m_bUseDoubleGesture;

	// 固定检测帧率
	bool m_bFixSkipFrames;

	// 卡尔曼滤波器
	JointCoordFilter* m_pFilter;

	// 模型文件路径
	string m_sModelDir;

	// 最大检测手个数限制
	int m_nMaxNumHand;
	vector<HandInfo> m_vHandResList;
	
	// 帧计数器
	long m_lFrameCount;
	int m_nDetSkipFrames;

	// 姿态模型配置
	int m_nPoseInputImgSize;
	int m_nPoseModelBackend;  // 推理设备 0-CPU 1-GPU
	bool m_bOnlyRightHand;
};

#endif // HAND_DETECT_IMPL_H