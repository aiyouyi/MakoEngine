#include "OrthographicEstimateCamera.h"
#include "glm/gtc/matrix_transform.hpp"
//#include "glm/glm.hpp"
#include "unsupported/Eigen/NonLinearOptimization"
#include "Eigen/Geometry"

namespace mt3dface
{
OrthographicEstimateCamera::OrthographicEstimateCamera()
{
	
}


OrthographicEstimateCamera::~OrthographicEstimateCamera()
{
}

/**
* @brief Generic functor for Eigen's optimisation algorithms.
*/
template<typename _Scalar, int NX = Eigen::Dynamic, int NY = Eigen::Dynamic>
struct Functor
{
	typedef _Scalar Scalar;
	enum {
		InputsAtCompileTime = NX,
		ValuesAtCompileTime = NY
	};
	typedef Eigen::Matrix<Scalar, InputsAtCompileTime, 1> InputType;
	typedef Eigen::Matrix<Scalar, ValuesAtCompileTime, 1> ValueType;
	typedef Eigen::Matrix<Scalar, ValuesAtCompileTime, InputsAtCompileTime> JacobianType;

	const int m_inputs, m_values;

    
	Functor() : m_inputs(InputsAtCompileTime), m_values(ValuesAtCompileTime) {}
	Functor(int inputs, int values) : m_inputs(inputs), m_values(values) {}

	int inputs() const { return m_inputs; }
	int values() const { return m_values; }
};

/**
* @brief LevenbergMarquardt cost function for the orthographic camera estimation.
*/
struct OrthographicParameterProjectionTemp : Functor<float>
{
public:
	// Creates a new OrthographicParameterProjectionTemp object with given data.
	OrthographicParameterProjectionTemp(float *image_points, float*model_points, int nNumsOfPoints,float *point_weight, int width, int height)
		: Functor<float>(6, nNumsOfPoints), image_points(image_points), model_points(model_points), 
		  point_weight(point_weight),m_nNumsOfPoints(nNumsOfPoints), width(width), height(height) {};


	// x = current params, fvec = the errors/differences of the proj with current params and the GT (image_points)
	int operator()(const Eigen::VectorXf& x, Eigen::VectorXf& fvec) const
	{
		auto rot_mtx_x = glm::rotate(glm::mat4(1.0f), (float)x[0], glm::vec3( 1.0f, 0.0f, 0.0f ));
		auto rot_mtx_y = glm::rotate(glm::mat4(1.0f), (float)x[1], glm::vec3( 0.0f, 1.0f, 0.0f ));
		auto rot_mtx_z = glm::rotate(glm::mat4(1.0f), (float)x[2], glm::vec3( 0.0f, 0.0f, 1.0f ));
		auto t_mtx = glm::translate(glm::mat4(1.0f), glm::vec3( (float)x[3], (float)x[4], 0.0f )); // glm: Col-major memory layout. [] gives the column

		// Note/Todo: Is this the full ortho? n/f missing? or do we need to multiply it with Proj...? See Shirley CG!
		// glm::frustum()?
		const float aspect = static_cast<float>(width) / height;
		auto ortho_mtx = glm::ortho(-1.0f * aspect * (float)x[5], 1.0f * aspect *(float)x[5], -1.0f *  (float)x[5], 1.0f *  (float)x[5]);
		auto model = t_mtx * rot_mtx_z * rot_mtx_x * rot_mtx_y;
		glm::vec4 viewport(0, height, width, -height); // flips y, origin top-left, like in OpenCV
		// P = RPY * P
		for (int i = 0; i < m_nNumsOfPoints; i++)
		{
			int i3 = i * 3;
			glm::vec3 point_3d(model_points[i3], model_points[i3 + 1], model_points[i3 + 2]);
			// projection given current params x:
			glm::vec3 proj_with_current_param_esti = glm::project(point_3d, model, ortho_mtx, viewport);
			// 			cv::Vec2f proj_point_2d(proj_with_current_param_esti.x, proj_with_current_param_esti.y);
			// 			cv::Vec2f point_2d(image_points[i << 1], image_points[i << 1 | 1]);
			// 			// diff of current proj to ground truth, our error
			// 			auto diff = cv::norm(proj_point_2d, point_2d);
			float dx = image_points[i << 1] - proj_with_current_param_esti.x;
			float dy = image_points[i << 1 | 1] - proj_with_current_param_esti.y;
			auto diff = dx*dx + dy*dy;
			// fvec should contain the differences
			// don't square it.
			diff *= point_weight[i];
			fvec[i] = diff;
		}
		return 0;
	};

private:
	float *image_points;
	float *model_points;
	float *point_weight;
	int m_nNumsOfPoints;
	int width;
	int height;
};

void OrthographicEstimateCamera::EstimateProMat(float* pModelPoints, float* pImagePoints, int nNumsOfPoints, int nWidth, int nHeight, float* pProjectionMat, float *pPointWieght, float* pCameraParm, bool isInit)
{
	const float aspect = static_cast<float>(nWidth) / nHeight;
	int num_params = 6;
	Eigen::VectorXf parameters; // [rot_x_pitch, rot_y_yaw, rot_z_roll, t_x, t_y, frustum_scale]
	parameters.setConstant(num_params, 0.0); // Set all 6 values to zero (except frustum_scale, see next line)
	parameters[5] = 1.5f; // This is just a rough hand-chosen scaling estimate - we could do a lot better. But it works.
	if (isInit&&pCameraParm!=NULL)
	{
		for (int i = 0; i < 3; i++)
		{
			parameters[i] = pCameraParm[i] * 3.14159f / 180.f;
		}
		for (int i = 3; i < 6; i++)
		{
			parameters[i] = pCameraParm[i];
		}
	}
	
	OrthographicParameterProjectionTemp cost_function(pImagePoints, pModelPoints, nNumsOfPoints,pPointWieght, nWidth, nHeight);
	Eigen::NumericalDiff<OrthographicParameterProjectionTemp> cost_function_with_derivative(cost_function, 0.0001f);
	// I had to change the default value of epsfcn, it works well around 0.0001. It couldn't produce the derivative with the default, I guess the changes in the gradient were too small.
	Eigen::LevenbergMarquardt<Eigen::NumericalDiff<OrthographicParameterProjectionTemp>,float> lm(cost_function_with_derivative);
	lm.minimize(parameters); // we could or should use the return value

	if (pCameraParm != NULL)
	{
		for (int i = 0; i < 3; i++)
		{
			pCameraParm[i] = parameters[i] * 180.f / 3.14159f;
		}
		for (int i = 3; i < 6; i++)
		{
			pCameraParm[i] = parameters[i];
		}
	}
	// 'parameters' contains the solution now.
	float l = -1.0f * aspect * static_cast<float>(parameters[5]);
	float r = 1.0f * aspect * static_cast<float>(parameters[5]);
	float t = -1.0f * static_cast<float>(parameters[5]);
	float b = 1.0f * static_cast<float>(parameters[5]);
	float r_x = parameters[0];
	float r_y = parameters[1];
	float r_z = parameters[2];
	float t_x = parameters[3];
	float t_y = parameters[4];
	auto rot_mtx_x = glm::rotate(glm::mat4(1.0f), r_x, glm::vec3( 1.0f, 0.0f, 0.0f ));
	auto rot_mtx_y = glm::rotate(glm::mat4(1.0f), r_y, glm::vec3( 0.0f, 1.0f, 0.0f ));
	auto rot_mtx_z = glm::rotate(glm::mat4(1.0f), r_z, glm::vec3( 0.0f, 0.0f, 1.0f ));
	auto t_mtx = glm::translate(glm::mat4(1.0f), glm::vec3( t_x, t_y, 0.0f ));
	auto modelview = t_mtx * rot_mtx_z * rot_mtx_x * rot_mtx_y;
	auto ortho_projection = glm::ortho(l, r, t, b);
	auto view_model = glm::transpose(modelview);
	float viewport_mat[16] = { nWidth*0.5f, 0.0f, 0.0f, nWidth*0.5f,
		                       0.0f, -nHeight*0.5f,0.0f,nHeight*0.5f,
							   0.0f, 0.0f, 1.0f, 0.0f,
							   0.0f, 0.0f, 0.0f, 1.0f };
	float full_projection_4x4[16];
	float *fMvp = (float*)&view_model[0];
	float *Op = (float*)&ortho_projection[0];
	for (int i = 0; i < 4;i++)
	{
		float *fp = full_projection_4x4 + i * 4;
		float *VM = viewport_mat + i * 4;
		for (int j = 0; j < 4;j++)
		{
			fp[j] = VM[0] * fMvp[j] * Op[0] + VM[1] * fMvp[j + 4] * Op[5] + VM[2] * fMvp[j + 8] * Op[10] + VM[3] * fMvp[j + 12] * Op[15];
		}
	}

	memcpy(pProjectionMat, full_projection_4x4, 12 * sizeof(float));
}
}
