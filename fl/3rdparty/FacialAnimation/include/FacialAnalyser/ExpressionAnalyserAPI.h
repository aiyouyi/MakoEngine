#ifndef EXPRESSION_ANALYSER_API_H
#define EXPRESSION_ANALYSER_API_H

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#include <vector>
#include <string>

#if defined(_WIN32) && defined(ANALYSER_DLL)
	#ifdef ANALYSER_EXPORT
		#define ANALYSER_API __declspec(dllexport)
	#else
		#define ANALYSER_API __declspec(dllimport)
	#endif 
#else 
	#define ANALYSER_API
#endif 

namespace ExpressionAnalyser
{
	typedef struct CalibrationConfig
	{
		bool flagCalibration = false;
		bool flagTrain = false;
		bool flagSave = false;
		std::string outputPath = "";
		std::string inputPath = "";
	} CalibrationConfig;

	class ANALYSER_API Analyser
	{
	public:
		/**
		 * Initialize expression analyzer with config or model path
		 *
		 * @param data_or_path path of config file or face model
		 */
		Analyser(const std::string& data_or_path);

		/**
		 * Initialize expression analyzer with with bytes of face model
		 *
		 * @param data pointer to data
		 * @param length length of input bytes
		 * @param dataType use default value 0
		 */
		Analyser(const char* const data, unsigned int length, int dataType = 0);

		/**
		 * Release resources in pipeline
		 */
		~Analyser();

		/**
		 * Update face image and 2D/3D landmarks to solve expressions
		 *
		 * If the input source is 2D image and 2D landmarks, landmarkdDepth
		 * should be empty. The expressions solved with 3D landmarks are not good as
		 * which solved with 2D landmarks.
		 *
		 * @param frame color face image of BGR format
		 * @param landmarks2d coordinates of 2D landmarks with format x1,x2,...xn,y1,y2...yn
		 * @param landmarks3d coordinated of 3D landmarks, which can be empty
		 *
		 * @return 0 if success
		 */
		int update(const cv::Mat& frame,
			const std::vector<float>& landmarks2d,
			const std::vector<float>& landmarks3d = std::vector<float>(0));

		/**
		 * Change the default state(ON) of blink detector
		 */
		void setBlinkFusionState(bool blinkFusionState);

		/**
		 * Change the default state(ON) of tongue detector
		 */
		void setTongueFusionState(bool tongueFusionState);
		
		/**
		 * Change the default state(ON) of eye gaze detector
		 */
		void setGazeFusionState(bool gazeFusionState);

		/**
		 * Estimate face shape with specific frame landmarks
		 *
		 * @param landmarks2d coordinates of 2D landmarks with format x1,x2,...xn,y1,y2...yn
		 * @param landmarksDepth coordinated of 3D landmarks, which can be empty
		 *
		 * @return 0 if success
		 */
		int estimateShape(const std::vector<float>& landmarks2d,
			const std::vector<float>& landmarks3d = std::vector<float>(0));

		/**
		 * Get 52 expression coefficients in the range [0,1]
		 */
		const std::vector<float> &getExpressions() const;

		/**
		 * Get Euler angles of head rotation
		 *
		 * @return Euler angles format: x-pitch, y-yaw, z-roll, left hand, intrinsic
		 */
		const std::vector<float> &getRotation() const;

		/**
		 * Get 50 shape coefficients in the range [0,1]
		 */
		const std::vector<float> &getShape() const;

		/**
		 * Get 7 dimension head pose
		 *
		 * @return rotation vector: [rx ry rz], translation: [tx, ty, 0], scale: [s]
		 */
		const std::vector<float> &getHeadPose() const;

		/**
		 * Get 6 dimension tongue status
		 *
		 * @return 0-up, 1-front, 2-roll up, 3-roll down, 4-roll left, 5-roll right.
		 *   Status indices 1,4,5 are set by results of neural network, others are
		 *   set according to tongue blendshapes of avatar.
		 */
		const std::vector<float> &getTongueResults() const;

		/**
		 * Get pitch and yaw degree of eye gaze
		 */
		const std::vector<float> &getGazeDegree() const;

		/**
		 * Get eye gaze direction which is unit vector
		 */
		const std::vector<float> &getGazeDirection();

		const std::vector<std::pair<float, float>> &getProjLandmarks() const;


		//------------ Depth Interface ------------//

		int update(const std::vector<float>& landmarks2d,
			const cv::Mat& bgrImage,
			const cv::Mat& depthImage);

		// Set current frameset info, set camera extrinsic and intrinsic parameters, 
		void setCameraParameters(const std::vector<int>& intParams, const std::vector<float>& floatParams);

		// Exposed interface
		int calibrateFace(const cv::Mat& bgrImage,
			const cv::Mat& depthImage,
			const std::vector<float>& landmarks2d,
			const CalibrationConfig& config,
			const std::vector<int> label = std::vector<int>{ 1, -1 }
		);

		// Interaction function
		int updateProcessedLandmarksInform(const std::vector<std::array<int, 3>>& interpolateTriIndex,
			const std::vector<std::array<float, 3>>& interpolateTriWeight,
			const std::string& jsonFile = "",
			const std::vector<int>& reserved = std::vector<int>(0));

		const std::vector<float>& getRotation3D() const;
		const std::vector<float>& getHeadPose3D() const;
		const std::vector<std::pair<float, float>>& getRGBDCameraProjLandmarks() const;

		std::vector<float> getGeoFeature(const std::string &featureName, const int &index = 0);

	private:
		void* pipeline_;
	};
}

#endif 