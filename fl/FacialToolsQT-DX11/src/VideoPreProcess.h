#ifndef _VIDEO_PREPROCESS_H_
#define _VIDEO_PREPROCESS_H_

#include <string>
#include <vector>
#include <memory>
#include "DX11ImageFilterDef.h"
#include "opencv2/opencv.hpp"
#include "BaseDefine/Vectors.h"

class VideoCaptureMapper;

class VideoPreProcess
{
public:
	VideoPreProcess();
	~VideoPreProcess(){}
public:
	void ccSavePng(const char* pFile, unsigned char* pBuffer, int x, int y, int comp);
	bool ProcessRectForName(const std::string& input_stream, const std::string& output_json_path);
	bool ProcessMask(const std::string& input_stream, const std::string& output_json_path);
	bool ProcessMaskWarp(const std::string& input_stream, const std::string& output_json_path);
	void RectPointSort(Vector2* rectPoint, Vector2* sortRect);
private:
	std::shared_ptr<VideoCaptureMapper> video_mapper;
	std::vector<int> blueValue;

	std::vector<std::string>blueIsRect;

	std::vector<cv::Vec4i> hierarchy;
	std::vector<std::vector<cv::Point>> contours;

};

#endif