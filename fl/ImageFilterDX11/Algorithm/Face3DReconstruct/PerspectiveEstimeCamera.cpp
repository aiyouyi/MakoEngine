#include "PerspectiveEstimeCamera.h"
#include "glm/gtc/matrix_transform.hpp"
//#include "glm/glm.hpp"
#include "unsupported/Eigen/NonLinearOptimization"
#include "Eigen/Geometry"
namespace mt3dface
{
    
void ProjectPoint(float *p, float *ProjectViewMat, float*out,float nWidth,int nHeight)
{
	float x = ProjectViewMat[0] * p[0] + ProjectViewMat[1] * p[1] + ProjectViewMat[2] * p[2] + ProjectViewMat[3];
	float y = ProjectViewMat[4] * p[0] + ProjectViewMat[5] * p[1] + ProjectViewMat[6] * p[2] + ProjectViewMat[7];
	float w = ProjectViewMat[12] * p[0] + ProjectViewMat[13] * p[1] + ProjectViewMat[14] * p[2] + ProjectViewMat[15];
	out[0] = (0.5f*(x / w) + 0.5f)*nWidth;
	out[1] = (0.5f*(y / w) + 0.5f)*nHeight;
}


template<typename _Scalar, int NX = Eigen::Dynamic, int NY = Eigen::Dynamic>
struct PerspectiveFunctor
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

	PerspectiveFunctor() : m_inputs(InputsAtCompileTime), m_values(ValuesAtCompileTime) {}
	PerspectiveFunctor(int inputs, int values) : m_inputs(inputs), m_values(values) {}

	int inputs() const { return m_inputs; }
	int values() const { return m_values; }
};

struct PerspectiveParameterProjection : PerspectiveFunctor<float>
{
public:
	// Creates a new OrthographicParameterProjection object with given data.
	PerspectiveParameterProjection(float *image_points, float*model_points, int nNumsOfPoints, float *point_weight,
		glm::mat4 &projectMat,int width, int height,float *pLastParam)
		: PerspectiveFunctor<float>(6, nNumsOfPoints), image_points(image_points), model_points(model_points), projectMat(projectMat),
		point_weight(point_weight), m_nNumsOfPoints(nNumsOfPoints), width(width), height(height) ,pLastParam(pLastParam){};


	// x = current params, fvec = the errors/differences of the proj with current params and the GT (image_points)
	int operator()(const Eigen::VectorXf& x, Eigen::VectorXf& fvec) const
	{
		auto rot_mtx_x = glm::rotate(glm::mat4(1.0f), x[0], glm::vec3( 1.0f, 0.0f, 0.0f ));
		auto rot_mtx_y = glm::rotate(glm::mat4(1.0f), x[1], glm::vec3( 0.0f, 1.0f, 0.0f ));
		auto rot_mtx_z = glm::rotate(glm::mat4(1.0f), x[2], glm::vec3( 0.0f, 0.0f, 1.0f ));
		auto t_mtx = glm::translate(glm::mat4(1.0f), glm::vec3( x[3], x[4], x[5] )); // glm: Col-major memory layout. [] gives the column
		auto projectView = projectMat*t_mtx*rot_mtx_y*rot_mtx_x*rot_mtx_z;
		projectView = glm::transpose(projectView);
		float proj_with_current_param_esti[2];
		float *projectViewMat = &projectView[0][0];
		float rx = x[0] * 180 / 3.1415926 - pLastParam[0];
		float ry = x[1] * 180 / 3.1415926 - pLastParam[1];
		float rz = x[2] * 180 / 3.1415926 - pLastParam[2];
		float lastDist = rx * rx + ry * ry + rz * rz;

		float lastCost = lastDist * pow(100.f, 1.0-pLastParam[3]);
// 		printf("lastCost = %f\n", lastCost);
// 		printf("lastDist = %f\n", lastDist);
		for (int i = 0; i < m_nNumsOfPoints; i++)
		{
			ProjectPoint(model_points + i * 3, projectViewMat, proj_with_current_param_esti, width, height);
			float dx = image_points[i << 1] - proj_with_current_param_esti[0];
			float dy = image_points[i << 1 | 1] - proj_with_current_param_esti[1];
			auto diff = dx*dx + dy*dy;
			diff *= point_weight[i];
			fvec[i] = diff +lastCost;
		//	sumdist += diff;
			
		}


	//	printf("sumdist = %f", sumdist);
		return 0;
	};

private:
	float *image_points;
	float *model_points;
	float *point_weight;
	glm::mat4 projectMat;
	int m_nNumsOfPoints;
	int width;
	int height;

	float *pLastParam;
};

PerspectiveEstimeCamera::PerspectiveEstimeCamera()
{
	memset(m_StablePoint, 0, 24 * sizeof(float));
}


PerspectiveEstimeCamera::~PerspectiveEstimeCamera()
{
}

void PerspectiveEstimeCamera::EstimateProMat(float* pModelPoints, float* pImagePoints, int nNumsOfPoints, int nWidth, int nHeight, float* pProjectionMat, float *pPointWieght, float* pCameraParm, bool isInit,float FovAngle)
{
	float sumStablePoint = 0.0;
	float *pPoint = pImagePoints + 51 * 2;
	for (int i=0;i<12;i++)
	{
		float x = pPoint[i * 2] - m_StablePoint[i * 2];
		float y = pPoint[i * 2 + 1] - m_StablePoint[i * 2 + 1];

		x /= nWidth;
		y /= nHeight;
		sumStablePoint += sqrt(x*x + y * y);
	}
	sumStablePoint /= 12;
	sumStablePoint *= 10000.f;
	//printf("sumStablePoint = %f\n", sumStablePoint);

	const float aspect = static_cast<float>(nWidth) / nHeight;
	int num_params = 6;
	Eigen::VectorXf parameters; // [rot_x_pitch, rot_y_yaw, rot_z_roll, t_x, t_y, t_z]
	parameters.setConstant(num_params, 0.0); 
	float fovAngle = FovAngle;
	float fovy = fovAngle*3.14159f / 180.f;
	parameters[5] = -1.0f / tan(fovy * 0.5);
	if (isInit&&pCameraParm != NULL)
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

	float pLastParam[4];
	pLastParam[0] = pCameraParm[0];
	pLastParam[1] = pCameraParm[1];
	pLastParam[2] = pCameraParm[2];
	pLastParam[3] = sumStablePoint;

	glm::mat4 project;

	m_CameraParams.fovy = fovAngle *3.14159f / 180.f;
	m_CameraParams.focalLength = 1.0 / tan(0.5 * fovy);
	m_CameraParams.far2 = 100.f;
	m_CameraParams.nWidth = nWidth;
	m_CameraParams.nHeight = nHeight;
	m_CameraParams.near2 = 0.01f;// 1.0f / tan(fovy * 0.5);
	project = glm::perspective(m_CameraParams.fovy, nWidth*1.f / nHeight, m_CameraParams.near2, m_CameraParams.far2);

	PerspectiveParameterProjection cost_function(pImagePoints, pModelPoints, nNumsOfPoints, pPointWieght,project,nWidth, nHeight,pLastParam);
	Eigen::NumericalDiff<PerspectiveParameterProjection> cost_function_with_derivative(cost_function, 0.00001f);
	Eigen::LevenbergMarquardt<Eigen::NumericalDiff<PerspectiveParameterProjection>, float> lm(cost_function_with_derivative);
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
	auto rot_mtx_x = glm::rotate(glm::mat4(1.0f), parameters[0], glm::vec3( 1.0f, 0.0f, 0.0f ));
	auto rot_mtx_y = glm::rotate(glm::mat4(1.0f), parameters[1], glm::vec3( 0.0f, 1.0f, 0.0f ));
	auto rot_mtx_z = glm::rotate(glm::mat4(1.0f), parameters[2], glm::vec3( 0.0f, 0.0f, 1.0f ));
	auto t_mtx = glm::translate(glm::mat4(1.0f), glm::vec3( parameters[3], parameters[4], parameters[5] ));
	auto projectView = project*t_mtx*rot_mtx_y*rot_mtx_x*rot_mtx_z;
	projectView = glm::transpose(projectView);
	float *full_projection_4x4 = &projectView[0][0];
	memcpy(pProjectionMat, full_projection_4x4, 16 * sizeof(float));

	memcpy(m_StablePoint, pImagePoints + 51 * 2,24 * sizeof(float));
}
}
