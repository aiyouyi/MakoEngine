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

	//------------------------------------ Detect�ӿ� ------------------------------------//
	bool init(const char* szModelPath, int deviceMode);
	void uninit();
	void reset();
	bool detect(const unsigned char* pBuffer, int width, int height, int stride, int ctType);
	ccHandRes getResult();
	ccGestureRes getGestureResult();

	//------------------------------------ ROI�ӿ� ------------------------------------//
	void setMaxHandNum(int numHand);
	void setRoiDetectSkipFrames(int numSkipFrames);

	//------------------------------------ Pose�ӿ� ------------------------------------//
	bool activateHandPose();
	void deactivateHandPose();
	void setPoseSmoothWeight(float value);
	void setOnlyRightHand(bool value);

	//------------------------------------ Gesture�ӿ� ------------------------------------//
	// ��������
	bool activateSingleGesture();
	void deactivateSingleGesture();
	void setSingleGestureRelaxation(float value);

	// ˫������
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
	// ��С�ɼ�������
	static const int m_cMinHandSize = 1000;

	// ����ͼ����̱�����
	const int m_cMinImgEdge = 50;

	// �����
	HandDetector* m_pHandDetector;
	HandPoseModel* m_pHandposeModel;
	SingleGestureModel* m_pSingleGestureModel;
	DoubleGestureModel* m_pDoubleGestureModel;

	// �����
	ccHandRes m_handRes;
	ccGestureRes m_gestureRes;

	// ģ�鿪��
	bool m_bUsePose;
	bool m_bUseSingleGesture;
	bool m_bUseDoubleGesture;

	// �̶����֡��
	bool m_bFixSkipFrames;

	// �������˲���
	JointCoordFilter* m_pFilter;

	// ģ���ļ�·��
	string m_sModelDir;

	// ������ָ�������
	int m_nMaxNumHand;
	vector<HandInfo> m_vHandResList;
	
	// ֡������
	long m_lFrameCount;
	int m_nDetSkipFrames;

	// ��̬ģ������
	int m_nPoseInputImgSize;
	int m_nPoseModelBackend;  // �����豸 0-CPU 1-GPU
	bool m_bOnlyRightHand;
};

#endif // HAND_DETECT_IMPL_H