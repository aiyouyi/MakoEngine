#ifndef rotation_h__
#define rotation_h__
namespace cvSolvePNP{
	// 用Rodrigues变换将旋转向量转换为旋转矩阵
	// @param rv: 旋转向量（3x1)
	// @param rm: 旋转矩阵（3x3)
	bool RodriguesV2M(double* rv, double* rm);

	// 用Rodrigues逆变换将旋转矩阵转换为旋转向量
	// @param rm: 旋转矩阵（3x3)
	// @param rv: 旋转向量（3x1)
	 bool RodriguesM2V(double* rm, double* rv);
}
#endif // rotation_h__