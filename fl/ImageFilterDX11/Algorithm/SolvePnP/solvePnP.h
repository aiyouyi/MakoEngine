#ifndef solvePnP_h__
#define solvePnP_h__
namespace cvSolvePNP {
	// 根据图像二维点和对应的模型三维点对估计模型姿态（旋转+位移)
	// 输入参数：
	//     _count： 输入的对应点对数量
	//     _opoints： 模型三维点坐标（model_x, model_y, model_z）
	//     _ipoints： 图像上二维点坐标（pixel_x, pixel_y）
	//     _cameraMatrix： 相机的内参数矩阵， 若图像尺寸为（w, h），max = max(w, h)， 则可按下式构造一个估计的内参数矩阵
	//                     |  max, 0,   w / 2 |
	//                     |  0,   max, h / 2 |
	//                     |  0,   0,   1.0   |
	//  输出参数：
	//    _rvec： 姿态的旋转矩阵(3 x 1)
	//    _tvec： 姿态的位移向量(3 x 1)
	void __declspec(dllexport) SolvePnP(int _count, double* _opoints, double* _ipoints,
		double* _cameraMatrix,
		double* _rvec, double* _tvec, double* _weight = 0);
}

#endif // solvePnP_h__
