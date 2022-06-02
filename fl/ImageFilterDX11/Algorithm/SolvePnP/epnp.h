﻿/*EPnP algorithm described in:
%
%       Francesc Moreno-Noguer, Vincent Lepetit, Pascal Fua.
%       Accurate Non-Iterative O(n) Solution to the PnP Problem.
%       In Proceedings of ICCV, 2007.
%
% Copyright (C) <2007>  <Francesc Moreno-Noguer, Vincent Lepetit, Pascal Fua>
%
% This program is free software: you can redistribute it and/or modify
% it under the terms of the version 3 of the GNU General Public License
% as published by the Free Software Foundation.
%
% This program is distributed in the hope that it will be useful, but
% WITHOUT ANY WARRANTY; without even the implied warranty of
% MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
% General Public License for more details.
% You should have received a copy of the GNU General Public License
% along with this program. If not, see <http://www.gnu.org/licenses/>.
%
% Vincent Lepetit, CVLab-EPFL, January 2008.
*/

#ifndef epnp_h
#define epnp_h

namespace cvSolvePNP {
	struct CvmtMat;

	class  epnp {
	public:
		epnp(void);
		~epnp();

		void set_internal_parameters(const double uc, const double vc,
			const double fu, const double fv);

		void set_maximum_number_of_correspondences(const int n);
		void reset_correspondences(void);
		void add_correspondence(const double X, const double Y, const double Z,
			const double u, const double v,double w =1.0);

		void set_external_correspondences(int count, double* oPoints, double* iPoints);

		double compute_pose(double R[3][3], double T[3]);

		void relative_error(double & rot_err, double & transl_err,
			const double Rtrue[3][3], const double ttrue[3],
			const double Rest[3][3], const double test[3]);

		//void print_pose(const double R[3][3], const double t[3]);
		double reprojection_error(const double R[3][3], const double t[3]);

	private:
		void choose_control_points(void);
		void compute_barycentric_coordinates(void);
		void fill_M(CvmtMat * M, const int row, const double * alphas, const double u, const double v, const double w);
		void compute_ccs(const double * betas, const double * ut);
		void compute_pcs(void);

		void solve_for_sign(void);

		void find_betas_approx_1(const CvmtMat * L_6x10, const CvmtMat * Rho, double * betas);
		void find_betas_approx_2(const CvmtMat * L_6x10, const CvmtMat * Rho, double * betas);
		void find_betas_approx_3(const CvmtMat * L_6x10, const CvmtMat * Rho, double * betas);
		void qr_solve(CvmtMat * A, CvmtMat * b, CvmtMat * X);

		double dot(const double * v1, const double * v2);
		double dist2(const double * p1, const double * p2);

		void compute_rho(double * rho);
		void compute_L_6x10(const double * ut, double * l_6x10);

		void gauss_newton(const CvmtMat * L_6x10, const CvmtMat * Rho, double current_betas[4]);
		void compute_A_and_b_gauss_newton(const double * l_6x10, const double * rho,
			double cb[4], CvmtMat * A, CvmtMat * b);

		double compute_R_and_t(const double * ut, const double * betas,
			double R[3][3], double t[3]);

		void estimate_R_and_t(double R[3][3], double t[3]);

		void copy_R_and_t(const double R_dst[3][3], const double t_dst[3],
			double R_src[3][3], double t_src[3]);

		void mat_to_quat(const double R[3][3], double q[4]);


		double uc, vc, fu, fv;
		bool use_external_correspondences;
		double * pws, *us, *alphas, *pcs,*pweight;
		int maximum_number_of_correspondences;
		int number_of_correspondences;

		double cws[4][3], ccs[4][3];
		double cws_determinant;
	};
}//mlab
#endif
