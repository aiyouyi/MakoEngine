#pragma once
#include <iostream>
#include <string>
#include "BaseDefine/Define.h"
#include "BaseDefine/Vectors.h"
#include <vector>
#include "FaceDetectorInterface.h"
#include "opencv2/opencv.hpp"


class FaceDetector
{
public:

	FaceDetector();
	~FaceDetector();

	void getFaceRes(cv::Mat &img);

	void ShowImg(cv::Mat &img);
	void DrawPoints(cv::Mat &img);

	Vector2 CalVerticalWithLength(Vector2 pos1, Vector2 pos2, float length);
	
	void EyeInterLRNew(Vector2 *pEyePointSrc, Vector2 *pPointDst, float direct);
	
	Vector2 *pEyePointDst;

	ccFDFaceRes *m_faceRes;

private:
	cc_handle_t m_handle;

};















//while (true)
//{	
//	cap >> frame;
//	if (!frame.empty())
//	{
//		break;
//	}		
//}