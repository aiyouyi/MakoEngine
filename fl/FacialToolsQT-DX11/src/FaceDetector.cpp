#include "FaceDetector.h"


FaceDetector:: FaceDetector()
{
	m_faceRes = NULL;
	pEyePointDst = NULL;
	
	m_handle = ccFDInit("../3rdparty/FaSDK/faceModel");
	if (!m_handle)
	{
		std::cout << "load model failed!" << std::endl;
		return;
	}
	ccActivateStageTwo(m_handle,"../3rdparty/FaSDK/faceModel");
	ccActivateStageTwoEye(m_handle,"../3rdparty/FaSDK/faceModel");
	ccFDReset(m_handle);
	ccFDSetLandmarkFilter(m_handle,0.6);
}

FaceDetector::~FaceDetector()
{
	SAFE_DELETE(m_faceRes);
	SAFE_DELETE(pEyePointDst);
}


void FaceDetector::getFaceRes(cv::Mat &img)
{
	if (!m_handle)
	{
		return;
	}
	ccFDDetect(m_handle,(unsigned char*)img.ptr(), img.cols, img.rows, img.cols * 3, CT_BGR);

	if (m_faceRes == NULL) 
	{
		m_faceRes = new ccFDFaceRes();
	}
	ccFDGetFaceRes(m_handle,m_faceRes);

}

void FaceDetector::ShowImg(cv::Mat &img)
{
	cv::Mat show_img = img.clone();

	float pFacePoint106[106*2];
	
	for (int i = 0; i < m_faceRes->numFace; i++)
	{
		for (int j = 0; j < 106; j++)
		{
			ccFDPoint pt = m_faceRes->arrFace[i].arrShapeDense[j];

			int x = round(pt.x);
			int y = round(pt.y);
			pFacePoint106[2*j] = pt.x;
			pFacePoint106[2*j+1] = pt.y;
			cv::circle(show_img, cv::Point(x, y), 2.0, cv::Scalar(0, 0, 255), -1);
			//cv::putText(show_img, std::to_string(j), cv::Point(x, y), cv::FONT_HERSHEY_COMPLEX, 0.4, cv::Scalar(0, 0, 0), 1);		
		}
	}
	cv::imshow("show_img", show_img);
	//Vector2 * pFacePoint = (Vector2 *)pFacePoint106;
	//// draw left
	//Vector2 pLeftEye[8];
	//memcpy(pLeftEye, pFacePoint + 51, 8 * sizeof(Vector2));

	//pEyePointDst = new Vector2[140];

	//EyeInterLRNew(pLeftEye, pEyePointDst, -1);

	////draw right
	//Vector2 pRightEye[8] = {
	//pFacePoint[65],
	//pFacePoint[64],
	//pFacePoint[63],
	//pFacePoint[62],
	//pFacePoint[61],
	//pFacePoint[68],
	//pFacePoint[67],
	//pFacePoint[66],
	//};
	//EyeInterLRNew(pRightEye, pEyePointDst+70, 1);


	//float * p_face8;
	//p_face8 = (float*)pEyePointDst;
	//for (int j = 0; j < 140; j++)
	//{
	//	int x = round(p_face8[2*j]);
	//	int y = round(p_face8[2*j+1]);
	//	cv::circle(show_img, cv::Point(x, y), 2.0, cv::Scalar(0, 0, 255), -1);
 //       cv::putText(show_img, std::to_string(j), cv::Point(x, y), cv::FONT_HERSHEY_COMPLEX, 0.4, cv::Scalar(0, 0, 0), 1);
	//	if ((j > 36) && (j < 40))
	//	{
	//		cv::line(show_img, cv::Point(round(p_face8[2 * j]), round(p_face8[2 * j + 1])), cv::Point(round(p_face8[2 * (j+1)]), round(p_face8[2 * (j+1) + 1])), cv::Scalar(0, 0, 0), 2);
	//	}

	//	if ((j > 106) && (j < 110))
	//	{
	//		cv::line(show_img, cv::Point(round(p_face8[2 * j]), round(p_face8[2 * j + 1])), cv::Point(round(p_face8[2 * (j + 1)]), round(p_face8[2 * (j + 1) + 1])), cv::Scalar(0, 0, 0), 2);
	//	}

	//}
	//cv::imshow("show_img", show_img);
 //   cv::waitKey(0);
}

void FaceDetector::DrawPoints(cv::Mat &img)
{
	float pFacePoint106[130 * 2];

	for (int i = 0; i < m_faceRes->numFace; i++)
	{
		for (int j = 0; j < 130; j++)
		{
			ccFDPoint pt = m_faceRes->arrFace[i].arrShapeDense[j];

			int x = round(pt.x);
			int y = round(pt.y);
			pFacePoint106[2 * j] = pt.x;
			pFacePoint106[2 * j + 1] = pt.y;
			cv::circle(img, cv::Point(x, y), 2.0, cv::Scalar(255, 0, 0), -1);
			//cv::putText(show_img, std::to_string(j), cv::Point(x, y), cv::FONT_HERSHEY_COMPLEX, 0.4, cv::Scalar(0, 0, 0), 1);        
		}
	}

	//std::vector<float>m_FacePoint = {
	//   237.512, 505.69, 238.78, 535.325, 241.589, 564.557, 246.679, 593.841, 253.855, 622.862, 261.982, 651.313, 270.448, 679.361, 280.395, 707.025, 292.952, 733.639, 308.038, 758.492, 325.359, 782, 344.264, 804.389, 364.272, 825.611, 385.615, 845.591, 408.744, 862.865, 435.713, 874.609, 465.861, 878.523, 496.181, 874.546, 523.388, 862.818, 546.398, 845.557, 567.32, 825.379, 586.954, 803.767, 605.543, 780.766, 622.344, 756.359, 636.661, 730.558, 648.08, 703.038, 656.728, 674.645, 663.731, 645.982, 670.105, 617.103, 675.662, 587.905, 679.533, 558.492, 681.532, 529.061, 681.81, 499.305, 273.87, 467.504, 300.108, 437.292, 336.72, 427.773, 374.364, 428.392, 409.616, 436.114, 406.157, 458.721, 372.971, 454.375, 338.278, 453.26, 304.624, 458.253, 506.662, 434.32, 540.569, 426.475, 576.751, 425.467, 611.791, 433.814, 637.821, 462.017, 608.191, 454.543, 575.766, 450.518, 542.267, 451.963, 510.303, 456.583, 317.653, 516.935, 338.444, 503.527, 364.357, 500.045, 389.281, 507.613, 406.38, 525.654, 383.101, 529.304, 359.032, 531.377, 336.489, 526.693, 361.593, 513.809, 361.416, 515.999, 511.43, 524.464, 527.984, 505.799, 552.771, 498.063, 578.437, 501.176, 599.359, 513.836, 580.798, 524.121, 558.306, 529.265, 534.342, 527.603, 554.144, 511.456, 555.555, 513.482, 457.632, 505.564, 458.212, 550.522, 458.807, 595.263, 459.285, 639.388, 427.6, 518.164, 417.624, 616.765, 400.629, 646.66, 413.985, 665.794, 429.649, 661.841, 460.386, 675.985, 490.56, 661.008, 506.653, 664.884, 520.044, 645.512, 501.524, 615.789, 489.018, 517.214, 374.468, 719.504, 406.734, 715.22, 440.956, 711.362, 462.875, 714.104, 484.64, 710.403, 518.637, 712.547, 551.236, 715.637, 530.289, 745.286, 502.195, 767.955, 463.178, 777.489, 423.901, 770.026, 395.38, 748.37, 385.069, 722.811, 410.018, 723.869, 462.977, 727.515, 515.854, 721.59, 540.516, 719.73, 518.413, 735.86, 462.794, 751.257, 407.307, 738.348, 439.478, 725.819, 486.695, 724.606, 433.888, 747.663, 492.028, 746.267, 390.39, 716.874, 423.4, 713.047, 502.013, 711.146, 534.981, 713.58, 341.53, 514.347, 381.682, 513.223, 533.985, 511.337, 574.248, 511.526, 346.992, 499.911, 360.952, 493.676, 375.273, 499.122, 376.246, 527.369, 362.337, 533.831, 347.922, 528.25, 539.958, 497.132, 554.18, 491.357, 568.327, 497.319, 568.337, 525.521, 554.099, 531.456, 539.944, 525.334,
	//};
	//std::vector<float>m_FacePoint1 = {
 //       444.667, 650.58, 445.172, 689.256, 447.824, 727.525, 453.435, 765.692, 461.807, 803.398, 471.461, 840.534, 481.455, 877.634, 493.611, 914.387, 510.168, 949.922, 531.162, 982.795, 556.14, 1013.41, 584.15, 1042.06, 614.428, 1068.54, 646.754, 1093.08, 681.554, 1113.98, 721.185, 1127.41, 764.084, 1131.24, 806.013, 1125.66, 843.228, 1110.22, 874.766, 1087.45, 903.272, 1061.15, 929.66, 1033.13, 953.924, 1002.93, 975.28, 970.758, 993.085, 936.68, 1007.03, 900.438, 1017.87, 863.623, 1027.78, 826.869, 1037.32, 789.884, 1044.88, 752.255, 1049.19, 714.165, 1050.5, 676.033, 1049.33, 637.449, 491.617, 601.743, 531.395, 568.273, 582.349, 564.392, 632.986, 571.958, 680.205, 586.312, 673.544, 613.279, 628.215, 603.629, 580.932, 596.771, 534.439, 596.486, 833.596, 583.23, 878.779, 567.157, 927.457, 557.499, 976.968, 558.983, 1015.82, 591.07, 974.751, 587.464, 929.82, 589.835, 884.375, 598.474, 841.008, 609.756, 554.746, 672.925, 581.618, 654.836, 614.072, 650.824, 645.437, 659.55, 666.467, 683.545, 638.174, 693.002, 608.493, 696.426, 579.169, 690.42, 614.119, 667.421, 609.95, 671.025, 846.211, 679.415, 866.747, 654.204, 898.434, 643.739, 931.617, 646.559, 959.312, 663.969, 935.127, 681.601, 905.622, 688.526, 875.393, 686.86, 903.355, 661.838, 903.345, 665.34, 757.435, 661.314, 758.26, 720.505, 759.116, 779.126, 759.889, 836.516, 710.193, 677.428, 699.332, 810.24, 677.61, 851.851, 698.753, 877.095, 718.624, 867.194, 759.415, 884.462, 799.336, 866.203, 818.144, 875.184, 837.859, 849.143, 816.458, 808.743, 803.752, 675.802, 657.309, 959.406, 690.946, 948.595, 728.716, 940.268, 755.566, 946.77, 782.695, 940.29, 821.992, 948.713, 858.006, 958.983, 832.796, 985.52, 799.222, 1004.2, 756.596, 1009.9, 714.656, 1003.54, 682.213, 985.188, 669.097, 961.773, 696.825, 964.476, 755.76, 970.395, 816.809, 964.528, 845.559, 961.472, 817.088, 964.971, 755.727, 970.96, 696.444, 964.971, 727.456, 966.916, 784.072, 967.124, 725.802, 967.637, 786.578, 967.764, 673.732, 953.679, 708.965, 943.5, 803.169, 943.656, 840.204, 953.548, 585.85, 667.993, 642.515, 666.883, 875.216, 662.3, 931.403, 661.411, 593.734, 649.088, 613.536, 640.991, 633.724, 648.265, 634.608, 685.472, 614.837, 693.824, 594.591, 686.313, 883.236, 643.604, 902.943, 635.735, 922.896, 643.08, 923.441, 679.831, 903.806, 687.902, 883.866, 680.424,
	//};
	//for (int i = 0; i < m_faceRes->numFace; i++)
	//{
	//	for (int j = 0; j < 130; j++)
	//	{
	//		ccFDPoint pt = m_faceRes->arrFace[i].arrShapeDense[j];

	//		int x = round(m_FacePoint1[2*j]);
	//		int y = round(m_FacePoint1[2 * j + 1]);

	//		cv::circle(img, cv::Point(x, y), 2.0, cv::Scalar(255, 0, 0), -1);
	//		//cv::putText(show_img, std::to_string(j), cv::Point(x, y), cv::FONT_HERSHEY_COMPLEX, 0.4, cv::Scalar(0, 0, 0), 1);	
	//	}
	//}

}

void FaceDetector::EyeInterLRNew(Vector2 *pEyePointSrc, Vector2 *pPointDst, float direct)
{
	for (int i = 0; i < 4; i++) {
		pPointDst[i * 2] = pEyePointSrc[i];
		pPointDst[i * 2 + 1] = (pEyePointSrc[i] + pEyePointSrc[i + 1])*0.5;
	}
	memcpy(pPointDst + 8, pEyePointSrc + 4, 4 * sizeof(Vector2));

	Vector2 StablePoint[7];
	StablePoint[0] = (pPointDst[0] + pPointDst[8])*0.5;
	StablePoint[1] = (pPointDst[0] + StablePoint[0])*0.5;
	StablePoint[2] = (pPointDst[8] + StablePoint[1])*0.5;

	float length0 = StablePoint[0].distance(pPointDst[0]);
	StablePoint[3] = Vector2(StablePoint[0].x, (StablePoint[0].y - length0));
	StablePoint[4] = Vector2(StablePoint[0].x, (StablePoint[0].y + length0));
	StablePoint[5] = Vector2(StablePoint[0].x, (StablePoint[0].y + length0 * 0.5));
	StablePoint[6] = Vector2(StablePoint[0].x, (StablePoint[0].y - length0 * 0.5));
	//memcpy(pPointDst + 12, StablePoint, 5 * sizeof(Vector2));

	Vector2 *pEyePoint = pPointDst;
	float avgLength = 0.0;
	for (int i = 9; i < 12; ++i) {
		avgLength += pEyePoint[i].distance(pEyePoint[i - 1]);
	}
	avgLength /= 3;
	float length1 = direct * avgLength;

	for (int i = 12; i < 20; i++)
	{
		pEyePoint[i] = CalVerticalWithLength(pEyePoint[i - 12], pEyePoint[i - 11], length1 * 0.6);
	}

	pEyePoint[20] = CalVerticalWithLength(pEyePoint[8], StablePoint[0], length1 * 0.8);
	pEyePoint[21] = CalVerticalWithLength(pEyePoint[8], StablePoint[3], length1 * 1.6);
	pEyePoint[22] = CalVerticalWithLength(pEyePoint[0], StablePoint[3], -length1 * 2);
	pEyePoint[23] = CalVerticalWithLength(pEyePoint[0], StablePoint[6], -length1 * 1.2);

	pEyePoint[24] = CalVerticalWithLength(pEyePoint[8], StablePoint[3], -length1 * 0.8);
	pEyePoint[25] = CalVerticalWithLength(pEyePoint[8], pEyePoint[21], length1  * 0.8);
	pEyePoint[26] = CalVerticalWithLength(pEyePoint[0], StablePoint[0], -length1 * 0.6);
	pEyePoint[27] = CalVerticalWithLength(pEyePoint[0], pEyePoint[23], -length1 * 0.8);

	for (int i = 28; i < 35; i++)
	{
		pEyePoint[i] = CalVerticalWithLength(pEyePoint[i - 16], pEyePoint[i - 15], length1*0.7);
	}

	pEyePoint[35] = CalVerticalWithLength(pEyePoint[8], StablePoint[6], -length1 * 1.5);
	pEyePoint[36] = CalVerticalWithLength(pEyePoint[8], StablePoint[3], -length1 * 2);


	float length_corner = 0;
	length_corner = pEyePoint[0].distance(pEyePoint[8]);
	length_corner = direct * length_corner;

	pEyePoint[37] = CalVerticalWithLength(pEyePoint[0], StablePoint[3], length_corner * 2.0);
	pEyePoint[38] = CalVerticalWithLength(pEyePoint[0], pEyePoint[22], length_corner * 2.2);
	pEyePoint[39] = CalVerticalWithLength(pEyePoint[8], StablePoint[5], length_corner * 2.0);
	pEyePoint[40] = pEyePoint[37] + pEyePoint[39] - pEyePoint[38];

	pEyePoint[41] = CalVerticalWithLength(pEyePoint[0], StablePoint[6], length_corner);
	pEyePoint[42] = CalVerticalWithLength(pEyePoint[0], pEyePoint[8], length_corner);
	pEyePoint[43] = CalVerticalWithLength(pEyePoint[8], pEyePoint[22], -length_corner * 1.2);
	pEyePoint[44] = CalVerticalWithLength(pEyePoint[8], pEyePoint[26], -length_corner);

	pEyePoint[45] = CalVerticalWithLength(pEyePoint[8], pEyePoint[0], -length_corner * 1.4);
	pEyePoint[46] = (pEyePoint[36] + pEyePoint[45])*0.5;
	pEyePoint[47] = CalVerticalWithLength(pEyePoint[8], StablePoint[5], length_corner * 0.8);
	pEyePoint[48] = CalVerticalWithLength(pEyePoint[8], StablePoint[0], length_corner);

	pEyePoint[49] = CalVerticalWithLength(pEyePoint[8], pEyePoint[23], length_corner * 1.5);
	pEyePoint[50] = CalVerticalWithLength(pEyePoint[8], StablePoint[6], length_corner * 1.4);
	pEyePoint[51] = CalVerticalWithLength(pEyePoint[0], StablePoint[0], -length_corner * 1.2);
	pEyePoint[52] = CalVerticalWithLength(pEyePoint[0], pEyePoint[21], -length_corner * 1.2);

	pEyePoint[53] = CalVerticalWithLength(pEyePoint[0], pEyePoint[21], -length_corner * 1.6);
	pEyePoint[54] = CalVerticalWithLength(pEyePoint[0], pEyePoint[23], -length_corner * 1.0);

	pEyePoint[55] = CalVerticalWithLength(pEyePoint[0], pEyePoint[23], -length_corner * 0.5);
	pEyePoint[56] = CalVerticalWithLength(pEyePoint[0], StablePoint[3], length_corner * 1);
	pEyePoint[57] = CalVerticalWithLength(pEyePoint[0], StablePoint[3], length_corner * 1.5);

	for (int i = 58; i < 65; i++)
	{
		pEyePoint[i] = CalVerticalWithLength(pEyePoint[i - 30], pEyePoint[i - 29], length1*0.8);
	}

	pEyePoint[65] = CalVerticalWithLength(pEyePoint[8], StablePoint[0], length_corner*0.6);
	pEyePoint[66] = CalVerticalWithLength(pEyePoint[8], StablePoint[6], length_corner*0.8);
	pEyePoint[67] = CalVerticalWithLength(pEyePoint[0], StablePoint[6], -length_corner * 0.8);
	pEyePoint[68] = CalVerticalWithLength(pEyePoint[0], pEyePoint[21], -length_corner * 0.6);
	pEyePoint[69] = (pEyePoint[55] + pEyePoint[56])*0.5;
}


Vector2 FaceDetector::CalVerticalWithLength(Vector2 pos1, Vector2 pos2, float length) {

	Vector2 vec = pos2 - pos1;

	Vector2 vecN = Vector2(-vec.y, vec.x).normalize();
	return pos1 + length * vecN;
}