#include "SolvePnP.h"
#include "epnp.h"
#include "Rodrigues.h"

namespace cvSolvePNP {

	void __declspec(dllexport) SolvePnP(int _count, double* _opoints, double* _ipoints,
		double* _cameraMatrix,
		double* _rvec, double* _tvec,double* _weight )
	{
		// 使用EPnP算法，算法详情可以参考文献 EPnP An Accurate O(n) Solution to the PnP Problem
		epnp PnP;

		double uc = _cameraMatrix[3 * 0 + 2];
		double vc = _cameraMatrix[3 * 1 + 2];
		double fu = _cameraMatrix[3 * 0 + 0];
		double fv = _cameraMatrix[3 * 1 + 1];

		PnP.set_internal_parameters(uc, vc, fu, fv);
		PnP.set_maximum_number_of_correspondences(_count);
		PnP.reset_correspondences();

		for (int i = 0; i < _count; i++)
		{
			double w = 1.0;
			if (_weight!=0)
			{
				w = _weight[i];
			}
			PnP.add_correspondence(
				_opoints[3 * i + 0], _opoints[3 * i + 1], _opoints[3 * i + 2],
				_ipoints[2 * i + 0], _ipoints[2 * i + 1],w);
		}

		double R_est[3][3];
		PnP.compute_pose(R_est, _tvec);
		//LOGD("err1 = %.5f", err2);
		RodriguesM2V(&R_est[0][0], _rvec);
	}
}