#include "VideoPreProcess.h"
#include "EffectKernel/Video/VideoCaptureMapper.hpp"
#include <algorithm>
#include <list>
#include <fstream>
#include "json.hpp"
#include "BaseDefine/commonFunc.h"
//#define STB_IMAGE_WRITE_IMPLEMENTATION
//#include "stb_image_write.h"

using namespace nlohmann;



VideoPreProcess::VideoPreProcess()
{
	video_mapper = std::make_shared<VideoCaptureFFmpeg>();
}

void VideoPreProcess::ccSavePng(const char* pFile, unsigned char* pBuffer, int x, int y, int comp)
{
	//stbi_write_png(pFile, x, y, comp, pBuffer, x * comp);
}

bool VideoPreProcess::ProcessRectForName(const std::string& input_stream, const std::string& output_json_path)
{
	if (video_mapper->Open(input_stream.c_str()) != -1)
	{
		float fps = video_mapper->GetFps();
		float duration = video_mapper->GetDuration();

		video_mapper->Start();
		int Threshold = 32;

		unsigned char* pData = nullptr;
		int NumFrame = 0;
		json root;
		std::list<json> frameList;

		pData = video_mapper->GetNextFrame();
		while (pData != nullptr)
		{
			int width = video_mapper->Width();
			int height = video_mapper->Height();
			int halfWidth = width * 0.5;
			unsigned char* new_buffer = (unsigned char*)malloc(sizeof(unsigned char) * width * 0.5 * height * 4);
			int new_byte_width = width * 0.5 * 4;
			int new_byte_height = height;

			int minHeight = height, maxHeight = 0, minWidth = width * 4, maxWidth = 0;

			unsigned char* src_width_half_ptr = pData + new_byte_width;
			for (int i = 0; i < height; i++)
			{
				for (int j = new_byte_width; j < width * 4; j += 4)
				{
					int Bvalue = *src_width_half_ptr;
					int Gvalue = *(src_width_half_ptr + 1);
					int Rvalue = *(src_width_half_ptr + 2);
					if (0 < Bvalue && Bvalue < 35 && Gvalue > 220 && Rvalue > 220 )
					{
						minHeight = (std::min)(minHeight, i);
						maxHeight = (std::max)(maxHeight, i);

						minWidth = (std::min)(minWidth, j / 4);
						maxWidth = (std::max)(maxWidth, j / 4);
					}
					src_width_half_ptr = src_width_half_ptr + 4;
				}
				src_width_half_ptr = src_width_half_ptr + new_byte_width;
			}

			//minWidth = minWidth / 4.0f;
			//maxWidth = maxWidth / 4.0f;

			int save_min_width = minWidth;
			int save_max_width = maxWidth;
			int save_min_height = minHeight;
			int save_max_height = maxHeight;

			minWidth = minWidth - (width * 0.5f);
			maxWidth = maxWidth - (width * 0.5f);

			minHeight = (std::max)(minHeight, 0);
			maxHeight = (std::min)(maxHeight, height);
			minWidth = (std::max)(minWidth, 0);
			maxWidth = (std::min)(maxWidth, int(width * 0.5f));

			Vector2 rect[4];
			if (minHeight <= maxHeight && minWidth <= maxWidth)
			{
				rect[0] = Vector2(minWidth / (float)halfWidth, minHeight / (float)height);
				rect[1] = Vector2(maxWidth / (float)halfWidth, minHeight / (float)height);
				rect[2] = Vector2(minWidth / (float)halfWidth, maxHeight / (float)height);
				rect[3] = Vector2(maxWidth / (float)halfWidth, maxHeight / (float)height);

				json frameRect;
				frameRect["Frame"] = NumFrame;
				frameRect["Rect"] = { rect[0].x, rect[0].y, rect[1].x, rect[1].y, rect[2].x, rect[2].y, rect[3].x, rect[3].y };
				frameList.push_back(frameRect);

				//save
				int _image_width = save_max_width - save_min_width + 1;
				int _image_height = save_max_height - save_min_height + 1;
				unsigned char* dst = new unsigned char[_image_width * _image_height * 4];
				memset(dst, 0, _image_width * _image_height * 4);
				int offset = 0;
				int row_data_size = _image_width * 4 * sizeof(unsigned char);
				int width_offset = minWidth * 4 * sizeof(unsigned char);

				unsigned char* head = pData + minHeight * width * 4;
				for (int nrow = save_min_height; nrow < save_max_height; nrow++)
				{
					memcpy(dst + offset, head + (save_min_width * 4), row_data_size);
					offset += row_data_size;
					head = head + width * 4;
				}

				std::string pic_path = output_json_path + "/" + std::to_string(NumFrame) + ".png";
				//ccSavePng(pic_path.c_str(), dst, _image_width, _image_height, 4);
				delete[] dst;
			}

			free(new_buffer);

			NumFrame++;

			pData = video_mapper->GetNextFrame();
		}

		root["FrameRect"] = frameList;
		std::string out_path = output_json_path + "/a.json";
		std::ofstream of(out_path, std::ios_base::binary);
		std::string str_json = root.dump(2, ' ', false);
		of.write(str_json.c_str(), str_json.size());
		of.close();
	}

	return true;
}



bool VideoPreProcess::ProcessMask(const std::string& input_stream, const std::string& output_json_path)
{
	std::string jsonPath = output_json_path + "/mask.json";
	json root;
	std::ifstream ifile(jsonPath.c_str());
	ifile >> root;
	json blueMaskJson = root["BlueMask"];
	if (!blueMaskJson.is_null())
	{
		for (int ni = 0; ni < blueMaskJson.size(); ni++)
		{
			json maskJson = blueMaskJson[ni];
			int value = maskJson["value"].get<int>();
			blueValue.push_back(value);
		}
	}

	if (video_mapper->Open(input_stream.c_str()) != -1)
	{
		float fps = video_mapper->GetFps();
		float duration = video_mapper->GetDuration();

		video_mapper->Start();
		int Threshold = 32;

		unsigned char* pData = nullptr;
		int NumFrame = 0;
		json root;
		std::list<json> frameList;

		pData = video_mapper->GetNextFrame();
		int maskCount = blueValue.size(); //Mask数量
		while (pData != nullptr)
		{
			int width = video_mapper->Width();
			int height = video_mapper->Height();
			//int halfWidth = width * 0.5;
			unsigned char* new_buffer = (unsigned char*)malloc(sizeof(unsigned char) * width * height * 4);
			int new_byte_width = width * 4;
			int new_byte_height = height;

			//int minHeight = height, maxHeight = 0, minWidth = width, maxWidth = 0;
			std::vector<int> minHeight(maskCount, height);
			std::vector<int> maxHeight(maskCount, 0);
			std::vector<int> minWidth(maskCount, width);
			std::vector<int> maxWidth(maskCount, 0);

			bool HasMask = false;
			unsigned char* src_width_half_ptr = pData;
			for (int i = 0; i < height; i++)
			{
				for (int j = 0; j < width * 4; j += 4)
				{
					int value = *src_width_half_ptr;
					int valueG = *(src_width_half_ptr + 1);
					int valueR = *(src_width_half_ptr + 2);

					for (int k = 0; k < blueValue.size(); k++)
					{
						if ( blueValue[k] - 5 < value && value < blueValue[k] + 5 && valueR > 220)
						{
							minHeight[k] = (std::min)(minHeight[k], i);
							maxHeight[k] = (std::max)(maxHeight[k], i);

							minWidth[k] = (std::min)(minWidth[k], j / 4);
							maxWidth[k] = (std::max)(maxWidth[k], j / 4);
							HasMask = true;
						}
					}

					//if (185 < value && value < 195  && valueG > 240 && valueR > 240 )
					//{
					//	minHeight = (std::min)(minHeight, i);
					//	maxHeight = (std::max)(maxHeight, i);

					//	minWidth = (std::min)(minWidth, j / 4);
					//	maxWidth = (std::max)(maxWidth, j / 4);
					//}
					src_width_half_ptr = src_width_half_ptr + 4;
				}
			}

			//int save_min_width = minWidth;
			//int save_max_width = maxWidth;
			//int save_min_height = minHeight;
			//int save_max_height = maxHeight;

			//minHeight = (std::max)(minHeight, 0);
			//maxHeight = (std::min)(maxHeight, height);
			//minWidth = (std::max)(minWidth, 0);
			//maxWidth = (std::min)(maxWidth, width);

			if (HasMask)
			{
				json frameRect;
				frameRect["Frame"] = NumFrame;
				std::list<json> maskJsonList;
				for (int ni = 0; ni < maskCount; ni++)
				{
					if (minHeight[ni] <= maxHeight[ni] && minWidth[ni] <= maxWidth[ni])
					{
						Vector2 rect[4];
						rect[0] = Vector2(minWidth[ni] / (float)width, minHeight[ni] / (float)height);
						rect[1] = Vector2(maxWidth[ni] / (float)width, minHeight[ni] / (float)height);
						rect[2] = Vector2(minWidth[ni] / (float)width, maxHeight[ni] / (float)height);
						rect[3] = Vector2(maxWidth[ni] / (float)width, maxHeight[ni] / (float)height);
						json rectJson;
						rectJson["Rect"] = { rect[0].x, rect[0].y, rect[1].x, rect[1].y, rect[2].x, rect[2].y, rect[3].x, rect[3].y };
						rectJson["Index"] = ni;
						maskJsonList.push_back(rectJson);
					}
					frameRect["Mask"] = maskJsonList;
				}

				frameList.push_back(frameRect);
			}


			//Vector2 rect[4];
			//if (minHeight <= maxHeight && minWidth <= maxWidth)
			//{
			//	rect[0] = Vector2(minWidth / (float)width, minHeight / (float)height);
			//	rect[1] = Vector2(maxWidth / (float)width, minHeight / (float)height);
			//	rect[2] = Vector2(minWidth / (float)width, maxHeight / (float)height);
			//	rect[3] = Vector2(maxWidth / (float)width, maxHeight / (float)height);

			//	json frameRect;
			//	frameRect["Frame"] = NumFrame;
			//	frameRect["Rect"] = { rect[0].x, rect[0].y, rect[1].x, rect[1].y, rect[2].x, rect[2].y, rect[3].x, rect[3].y };
			//	frameList.push_back(frameRect);

			//	//save
			//	int _image_width = save_max_width - save_min_width + 1;
			//	int _image_height = save_max_height - save_min_height + 1;
			//	unsigned char* dst = new unsigned char[_image_width * _image_height * 4];
			//	memset(dst, 0, _image_width * _image_height * 4);
			//	int offset = 0;
			//	int row_data_size = _image_width * 4 * sizeof(unsigned char);
			//	int width_offset = minWidth * 4 * sizeof(unsigned char);

			//	unsigned char* head = pData + minHeight * width * 4;
			//	for (int nrow = save_min_height; nrow < save_max_height; nrow++)
			//	{
			//		memcpy(dst + offset, head + (save_min_width * 4), row_data_size);
			//		offset += row_data_size;
			//		head = head + width * 4;
			//	}

			//	std::string pic_path = output_json_path + "/" + std::to_string(NumFrame) + ".png";
			//	ccSavePng(pic_path.c_str(), dst, _image_width, _image_height, 4);
			//	delete[] dst;
			//}

			free(new_buffer);
			new_buffer = nullptr;

			NumFrame++;

			pData = video_mapper->GetNextFrame();
		}

		root["FrameRect"] = frameList;
		std::string out_path = output_json_path + "/a.json";
		std::ofstream of(char2TChar( out_path.c_str()), std::ios_base::binary);
		std::string str_json = root.dump(2, ' ', false);
		of.write(str_json.c_str(), str_json.size());
		of.close();
	}

	return true;
}

void VideoPreProcess::RectPointSort(Vector2* rectPoint, Vector2* sortRect)
{
	double longSide, shortSide;    //定义长短边
	Vector2 tempPoint;
	// 按x轴坐标从小到大排序
	for (int i = 0; i < 3; i++)
	{
		for (int j = i + 1; j < 4; j++)
		{
			if (rectPoint[i].x > rectPoint[j].x)
			{
				tempPoint = rectPoint[i];
				rectPoint[i] = rectPoint[j];
				rectPoint[j] = tempPoint;
			}
		}
	}

	// 按纵坐标排序，依次排为()： 0左上角   1右上角   2 左下 3右下角
	if (rectPoint[0].y > rectPoint[1].y)
	{
		tempPoint = rectPoint[0];
		rectPoint[0] = rectPoint[1];
		rectPoint[1] = tempPoint;
	}
	if (rectPoint[2].y > rectPoint[3].y)
	{
		tempPoint = rectPoint[2];
		rectPoint[2] = rectPoint[3];
		rectPoint[3] = tempPoint;
	}
	Vector2 temp = rectPoint[2];
	rectPoint[2] = rectPoint[1];
	rectPoint[1] = temp;

	// 计算长短边，适应两种矩形
	longSide = sqrt(pow(rectPoint[1].x - rectPoint[0].x, 2) + pow(rectPoint[1].y - rectPoint[0].y, 2));
	shortSide = sqrt(pow(rectPoint[2].x - rectPoint[0].x, 2) + pow(rectPoint[2].y - rectPoint[0].y, 2));
	if (longSide > shortSide)
	{
		for (int i = 0; i < 4; i++)
		{
			sortRect[i].x = rectPoint[i].x;
			sortRect[i].y = rectPoint[i].y;
		}
	}
	else
	{
		float dy = rectPoint[1].y - rectPoint[0].y;
		float dx = rectPoint[1].x - rectPoint[0].x;
		float k = dy / dx;////计算斜率
		if (k > 0)
		{
			sortRect[0] = rectPoint[2];
			sortRect[1] = rectPoint[0];
			sortRect[2] = rectPoint[3];
			sortRect[3] = rectPoint[1];
		}
		else
		{
			sortRect[0] = rectPoint[1];
			sortRect[1] = rectPoint[3];
			sortRect[2] = rectPoint[0];
			sortRect[3] = rectPoint[2];
		}
	}
}

bool VideoPreProcess::ProcessMaskWarp(const std::string& input_stream, const std::string& output_json_path)
{
	std::string jsonPath = output_json_path + "/mask.json";
	json root;
	std::ifstream ifile(char2TChar(jsonPath.c_str()));
	ifile >> root;
	json blueMaskJson = root["BlueMask"];
	if (!blueMaskJson.is_null())
	{
		blueValue.clear();
		blueIsRect.clear();
		for (int ni = 0; ni < blueMaskJson.size(); ni++)
		{
			json maskJson = blueMaskJson[ni];
			std::string isRect = maskJson["isrect"].get<std::string>();
			int value = maskJson["value"].get<int>();
			blueValue.push_back(value);
			blueIsRect.push_back(isRect);
		}
	}

	int maskCount = blueValue.size(); //Mask数量
	if (maskCount <= 0)
	{
		return false;
	}

	if (video_mapper->Open(input_stream.c_str()) != -1)
	{
		float fps = video_mapper->GetFps();
		float duration = video_mapper->GetDuration();

		video_mapper->Start();
		int Threshold = 32;

		unsigned char* pData = nullptr;
		int NumFrame = 0;
		json root;
		std::list<json> frameList;

		pData = video_mapper->GetNextFrame();

		int width = video_mapper->Width();
		int height = video_mapper->Height();
		
		Vector2 RectLeftTop;
		Vector2 RectLeftDown;
		Vector2 RectRightTop;
		Vector2 RectRightDown;

		Vector2 CircleLeftTop;
		Vector2  CircleLeftDown;
		Vector2  CircleRightTop;
		Vector2  CircleRightDown;

		while (pData != nullptr)
		{
			cv::Mat frame(height,width, CV_8UC4, pData);
			//cv::imshow("frame", frame);
			
			std::vector<cv::Mat> rgbChannels(4);
			cv::split(frame, rgbChannels);
			
			cv::Mat B_img = rgbChannels[0];
			//cv::imshow("B_img", B_img);
			cv::Mat G_img = rgbChannels[1];
			cv::Mat R_img = rgbChannels[2];
			bool HasMask = false;
			std::list<json> maskJsonList;



			for (int c = 0; c < maskCount; c++)
			{

				int thresh = blueValue[c];
				std::string isRect = blueIsRect[c];
				
				cv::Mat dst = cv::Mat::zeros(height,width, CV_8UC1);
				//cv::threshold(B_img,dst, 242, 255, CV_THRESH_BINARY);
				
				int low = (std::max)(3, thresh - 8);
				int up = (std::min)(thresh + 8, 255);
				//cv::inRange(B_img, low, up, dst);//B值在某个特定范围内，进行填充
				//图像遍历
				for (int row = 0; row < height; row++)
				{
					const uchar* srcGreenDada = G_img.ptr<uchar>(row);
					const uchar* srcRedDada = R_img.ptr<uchar>(row);
					const uchar* srcDada = B_img.ptr<uchar>(row);
					uchar* dstData = dst.ptr<uchar>(row);
					for (int col = 0; col < width; col++)
					{
						if (srcDada[col] >= low && srcDada[col] <= up && srcRedDada[col] > 200)
						{
							dstData[col] = 255;
						}
					}
				}

				cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5), cv::Point(-1, -1));
				//morphologyEx(dst, dst, cv::MORPH_OPEN, kernel);
				//morphologyEx(dst, dst, cv::MORPH_CLOSE, kernel);
				cv::imshow("dst", dst);

				//找最大轮廓的索引
				cv::Mat imageContour = cv::Mat::zeros(dst.size(), CV_8UC1);
				cv::findContours(dst, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);				

				float conArea = 0;
				int maxConAreaIndex = -1;
				for (int i = 0; i < contours.size(); i++)
				{
					if (contours[i].size() > 50)
					{
						float area = cv::contourArea(contours[i]);
						if (area > conArea)
						{
							conArea = area;
							maxConAreaIndex = i;
						}
					}

				}
				if (maxConAreaIndex < 0)
				{
					continue;
				}
				cv::drawContours(imageContour, contours, maxConAreaIndex, cv::Scalar(255), 1, 8, hierarchy);
				cv::imshow("imageContour", imageContour);
				

				Vector2 vertices[4];
				json rectJson;

				if (isRect == "2")//椭圆拟合
				{
					//椭圆拟合
					cv::RotatedRect box = cv::fitEllipse(contours[maxConAreaIndex]);

					//ellipse(imageContour, box, cv::Scalar(255), 1, CV_AA);
					cv::Point2f P[4];
					box.points(P);
					Vector2 rectPoint[4];
					for (int j = 0; j <= 3; j++)
					{
						cv::line(imageContour, P[j], P[(j + 1) % 4], cv::Scalar(255), 2);
						rectPoint[j].x = P[j].x;
						rectPoint[j].y = P[j].y;
					}
					//排序，左上 右上，左下，右下
					RectPointSort(rectPoint, vertices);

					for (int i = 0; i < 4; ++i)
					{
						cv::circle(imageContour, cv::Point((int)vertices[i].x, (int)vertices[i].y), 10, cv::Scalar(255), 3);
						cv::putText(imageContour, std::to_string(i), cv::Point((int)vertices[i].x, (int)vertices[i].y), 1, 2, cv::Scalar(255), 1, 8);
					}

					//归一化
					for (int i = 0; i < 4; i++)
					{
						vertices[i].x = vertices[i].x / width;
						vertices[i].y = vertices[i].y / height;

						char x_[10];
						char y_[10];
						sprintf(x_, "%.2f", vertices[i].x);
						sprintf(y_, "%.2f", vertices[i].y);

						vertices[i].x = atof(x_);
						vertices[i].y = atof(y_);
					}

					rectJson["Rect"] = { vertices[0].x, vertices[0].y, vertices[1].x, vertices[1].y, vertices[2].x, vertices[2].y, vertices[3].x, vertices[3].y };

				}
				else if (isRect == "1")
				{
					std::vector<std::vector<cv::Point>>polyContours(contours.size());
					cv::approxPolyDP(contours[maxConAreaIndex], polyContours[maxConAreaIndex], 10, true);//多边形包围轮廓

					cv::drawContours(imageContour, polyContours, maxConAreaIndex, cv::Scalar(255), 1, 8, hierarchy);

					//凸包找矩形角点
					std::vector<int> hull;
					cv::convexHull(polyContours[maxConAreaIndex], hull, false);//检测该轮廓的凸包


					//对于矩形来说角点是四个
					if (hull.size() == 4)
					{
						Vector2 rectPoint[4];
						for (int i = 0; i < hull.size(); ++i)
						{
							//cv::circle(imageContour, polyContours[maxConAreaIndex][i], 10, cv::Scalar(255), 3);
							//cv::putText(imageContour, std::to_string(i), polyContours[maxConAreaIndex][i], 1, 2, cv::Scalar(255), 1, 8);
							rectPoint[i].x = polyContours[maxConAreaIndex][i].x;
							rectPoint[i].y = polyContours[maxConAreaIndex][i].y;
						}
						//排序，左上 右上，左下，右下
						RectPointSort(rectPoint, vertices);


					}

					for (int i = 0; i < hull.size(); ++i)
					{
						cv::circle(imageContour, cv::Point((int)vertices[i].x, (int)vertices[i].y), 1, cv::Scalar(255), 3);
						cv::putText(imageContour, std::to_string(i), cv::Point((int)vertices[i].x, (int)vertices[i].y), 1, 1, cv::Scalar(255), 1, 8);
					}

					//归一化
					for (int i = 0; i < 4; i++)
					{
						vertices[i].x = vertices[i].x / width;
						vertices[i].y = vertices[i].y / height;

						char x_[10];
						char y_[10];
						sprintf(x_, "%.2f", vertices[i].x);
						sprintf(y_, "%.2f", vertices[i].y);

						vertices[i].x = atof(x_);
						vertices[i].y = atof(y_);
					}
					////结合上一帧判断当前帧角点位置是否需要改变
					//if (NumFrame == 168)
					//{
					//	RectLeftTop = vertices[0];
					//	RectRightDown = vertices[3];
					//}

					//bool x_LeftOffset = (abs(vertices[0].x - RectLeftTop.x)) < (5 * 1.0 / width) ? true : false;
					//if (x_LeftOffset)
					//{
					//	float temp = vertices[0].x;

					//	vertices[0].x = RectLeftTop.x;

					//	RectLeftTop.x = temp;
					//}
					//bool y_LeftOffset = (abs(vertices[0].y - RectLeftTop.y)) < (5 * 1.0 / height) ? true : false;
					//if (y_LeftOffset)
					//{
					//	float temp = vertices[0].y;

					//	vertices[0].y = RectLeftTop.y;

					//	RectLeftTop.y = temp;
					//}

					//bool x_RightOffset = (abs(vertices[3].x - RectRightDown.x)) < (5 * 1.0 / width) ? true : false;
					//if (x_RightOffset)
					//{
					//	float temp = vertices[3].x;

					//	vertices[3].x = RectRightDown.x;

					//	RectRightDown.x = temp;
					//}
					//bool y_RightOffset = (abs(vertices[3].y - RectRightDown.y)) < (5 * 1.0 / height) ? true : false;
					//if (y_RightOffset)
					//{
					//	float temp = vertices[3].y;

					//	vertices[3].y = RectRightDown.y;

					//	RectRightDown.y = temp;
					//}

					rectJson["Rect"] = { vertices[0].x, vertices[0].y, vertices[1].x, vertices[1].y, vertices[2].x, vertices[2].y, vertices[3].x, vertices[3].y };
				}

				else//非矩形 圆和椭圆找最小外接正矩形
				{


					//绘制轮廓的最小外结矩形
					//cv::RotatedRect rect = cv::minAreaRect(contours[maxConAreaIndex]);
					//cv::Point2f P[4];//初始化矩形四个顶点坐标
					//rect.points(P);
					//Vector2 rectPoint[4];
					//for (int j = 0; j <= 3; j++)
					//{
					//	cv::line(imageContour, P[j], P[(j + 1) % 4], cv::Scalar(255), 2);
					//	vertices[j].x = P[j].x;
					//	vertices[j].y = P[j].y;
					//}
					////排序，左上 右上，左下，右下
					///*RectPointSort(rectPoint, vertices);*/
					
					////绘制轮廓的最小外接正矩形
					cv::Rect boundRect = cv::boundingRect(cv::Mat(contours[maxConAreaIndex]));
					cv::rectangle(imageContour, cv::Point(boundRect.x, boundRect.y), cv::Point(boundRect.x + boundRect.width, boundRect.y + boundRect.height), cv::Scalar(255), 2, 8);
					vertices[0] = Vector2(boundRect.x, boundRect.y);
					vertices[1] = Vector2(boundRect.x + boundRect.width, boundRect.y);
					vertices[2] = Vector2(boundRect.x, boundRect.y + boundRect.height);
					vertices[3] = Vector2(boundRect.x + boundRect.width, boundRect.y + boundRect.height);

					for (int i = 0; i < 4; ++i)
					{
						cv::circle(imageContour, cv::Point((int)vertices[i].x, (int)vertices[i].y), 10, cv::Scalar(255), 3);
						cv::putText(imageContour, std::to_string(i), cv::Point((int)vertices[i].x, (int)vertices[i].y), 1, 2, cv::Scalar(255), 1, 8);
					}

					//归一化
					for (int i = 0; i < 4; i++)
					{
						vertices[i].x = vertices[i].x / width;
						vertices[i].y = vertices[i].y / height;

						char x_[10];
						char y_[10];
						sprintf(x_, "%.2f", vertices[i].x);
						sprintf(y_, "%.2f", vertices[i].y);

						vertices[i].x = atof(x_);
						vertices[i].y = atof(y_);
					}

					////结合上一帧判断当前帧角点位置是否需要改变
					//if (NumFrame == 168)
					//{
					//	CircleLeftTop = vertices[0];
					//	CircleRightDown = vertices[3];
					//}

					//bool x_LeftOffset = (abs(vertices[0].x - CircleLeftTop.x)) < (5*1.0/width) ? true : false;
					//if (x_LeftOffset)
					//{
					//	float temp = vertices[0].x;

					//	vertices[0].x = CircleLeftTop.x;

					//	CircleLeftTop.x = temp;
					//}
					//bool y_LeftOffset = (abs(vertices[0].y - CircleLeftTop.y)) < (5 * 1.0 / height) ? true : false;
					//if (y_LeftOffset)
					//{
					//	float temp = vertices[0].y;

					//	vertices[0].y = CircleLeftTop.y;

					//	CircleLeftTop.y = temp;
					//}

					//bool x_RightOffset = (abs(vertices[3].x - CircleRightDown.x)) < (5 * 1.0 / width) ? true : false;
					//if (x_RightOffset)
					//{
					//	float temp = vertices[3].x;

					//	vertices[3].x = CircleRightDown.x;

					//	CircleRightDown.x = temp;
					//}
					//bool y_RightOffset = (abs(vertices[3].y - CircleRightDown.y)) < (5 * 1.0 / height) ? true : false;
					//if (y_RightOffset)
					//{
					//	float temp = vertices[3].y;

					//	vertices[3].y = CircleRightDown.y;

					//	CircleRightDown.y = temp;
					//}


					rectJson["Rect"] = { vertices[0].x, vertices[0].y, vertices[1].x, vertices[1].y, vertices[2].x, vertices[2].y, vertices[3].x, vertices[3].y };
				}


				std::cout << "第" << NumFrame << "帧" << std::endl;
				//绘制轮廓的最小外结矩形
				//cv::RotatedRect rect = cv::minAreaRect(contours[maxConAreaIndex]);
				//cv::Point2f P[4];//初始化矩形四个顶点坐标
				//rect.points(P);
				//for (int j = 0; j <= 3; j++)
				//{
				//	cv::line(imageContour, P[j], P[(j + 1) % 4], cv::Scalar(255), 2);
				//}
				
				//调试显示结果
				cv::imshow("imageContour", imageContour);
			    cv:cvWaitKey(0);

				rectJson["Index"] = c;
				maskJsonList.push_back(rectJson);

				HasMask = true;
			}
			
			if (HasMask)
			{
				json frameRect;

				frameRect["Frame"] = NumFrame;
				frameRect["Mask"] = maskJsonList;

				frameList.push_back(frameRect);
			}

		   
			NumFrame++;

			pData = video_mapper->GetNextFrame();

		}

		root["FrameRect"] = frameList;
		std::string out_path = output_json_path + "/a.json";
		std::ofstream of(char2TChar(out_path.c_str()), std::ios_base::binary);
		std::string str_json = root.dump(2, ' ', false);
		of.write(str_json.c_str(), str_json.size());
		of.close();
	}
	return true;
}
