#include "Rodrigues.h"
#include "Eigen/SVD"
#include <limits>
using namespace Eigen;
namespace cvSolvePNP {
	bool eigenRodrigues(const MatrixXf& src, MatrixXf& dst)
	{
		if (src.rows() == 3 && src.cols() == 1)
		{
			//输入为旋转向量
			float rx, ry, rz, theta;
			rx = src(0, 0);
			ry = src(1, 0);
			rz = src(2, 0);
			theta = sqrt(rx*rx + ry*ry + rz*rz);
			if (theta < std::numeric_limits<float>::epsilon())
			{
				dst.setIdentity(3, 3);
			}
			else
			{
				const float I[] = { 1, 0, 0, 0, 1, 0, 0, 0, 1 };

				float c = cos(theta);
				float s = sin(theta);
				float c1 = 1.0f - c;
				float itheta = theta ? 1.0f / theta : 0.0f;

				rx *= itheta; ry *= itheta; rz *= itheta;

				float rrt[] = { rx*rx, rx*ry, rx*rz, rx*ry, ry*ry, ry*rz, rx*rz, ry*rz, rz*rz };
				float _r_x_[] = { 0, -rz, ry, rz, 0, -rx, -ry, rx, 0 };
				float R[9];

				// R = cos(theta)*I + (1 - cos(theta))*r*rT + sin(theta)*[r_x]
				// where [r_x] is [0 -rz ry; rz 0 -rx; -ry rx 0]
				for (int k = 0; k < 9; k++)
					R[k] = c*I[k] + c1*rrt[k] + s*_r_x_[k];

				dst = Map<MatrixXf>(R, 3, 3).transpose();
			}
		}
		else if (src.cols() == 3 && src.rows() == 3)//输入为旋转矩阵
		{
			float theta, s, c;
			float rx, ry, rz;
			JacobiSVD<MatrixXf> svd(src.transpose(), ComputeFullU | ComputeFullV);
			MatrixXf RMat = (svd.matrixU() * (svd.matrixV().transpose()));

			const float* R = RMat.data();
			rx = R[7] - R[5];
			ry = R[2] - R[6];
			rz = R[3] - R[1];

			s = sqrt((rx*rx + ry*ry + rz*rz)*0.25f);
			c = (R[0] + R[4] + R[8] - 1)*0.5f;
			c = c > 1.0f ? 1.0f : c < -1.0f ? -1.0f : c;
			theta = acos(c);

			if (s < 1e-5)
			{
				float t;

				if (c > 0)
					rx = ry = rz = 0;
				else
				{
					t = (R[0] + 1)*0.5f;
					rx = sqrt(std::max(t, 0.0f));
					t = (R[4] + 1)*0.5f;
					ry = sqrt(std::max(t, 0.0f))*(R[1] < 0 ? -1.0f : 1.0f);
					t = (R[8] + 1)*0.5f;
					rz = sqrt(std::max(t, 0.0f))*(R[2] < 0 ? -1.0f : 1.0f);
					if (fabs(rx) < fabs(ry) && fabs(rx) < fabs(rz) && (R[5] > 0) != (ry*rz > 0))
						rz = -rz;
					theta /= sqrt(rx*rx + ry*ry + rz*rz);
					rx *= theta;
					ry *= theta;
					rz *= theta;
				}//if( c > 0 )
			}
			else
			{
				float vth = 1 / (2 * s);
				vth *= theta;
				rx *= vth; ry *= vth; rz *= vth;
			}//if( s < 1e-5 )
			//赋值结果
			float rev[] = { rx, ry, rz };
			dst = Map<MatrixXf>(rev, 3, 1);
		}
		else
		{
			return false;
		}
		return true;
	}

	bool RodriguesV2M(double* rv, double* rm)
	{
		MatrixXf v(3, 1);
		MatrixXf m(3, 3);

		for (int i = 0; i < 3; i++)
			v(i) = static_cast<float>(rv[i]);

		bool rst = eigenRodrigues(v, m);

		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 3; j++)
			{
				rm[i * 3 + j] = m(i, j);
			}
		}
		return rst;
	}

	bool RodriguesM2V(double* rm, double* rv)
	{
		MatrixXf v(3, 1);
		MatrixXf m(3, 3);

		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 3; j++)
			{
				m(i, j) = static_cast<float>(rm[i * 3 + j]);
			}
		}

		bool rst = eigenRodrigues(m, v);

		for (int i = 0; i < 3; i++)
			rv[i] = v(i);

		return rst;

	}
}