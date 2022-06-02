#ifndef EXPRESSION_ANALYSER_API_C_H 
#define EXPRESSION_ANALYSER_API_C_H

#if defined(_WIN32) && defined(ANALYSER_DLL)
	#ifdef ANALYSER_EXPORT
		#define ANALYSER_API __declspec(dllexport)
	#else
		#define ANALYSER_API __declspec(dllimport)
	#endif 
#else 
	#define ANALYSER_API
#endif 

#ifdef __cplusplus
extern "C" {
#endif

typedef struct AnalyserResults
{
	float expressions[52];
	float rotation[3];
	float shape[50];
	float headPose[7];
	float tongue[6];
	float gazeDegree[2];
	float gazeDirection[3];
} AnalyserResult;

/**
 * Initialize expression analyzer with config or model path
 *
 * @param path path of config file or face model
 * @return handle for expression analyser instance, must be
 *   released before exit
 */
ANALYSER_API void* analyserInitPath(const char* path);

/**
 * Initialize expression analyzer with bytes of face model
 *
 * @param data pointer to data
 * @param length length of input bytes
 * @param dataType use default value 0
 */
ANALYSER_API void* analyserInitData(const char* const data, unsigned int length, int dataType = 0);

/**
 * Delete pointer to analyser instance
 */
ANALYSER_API void analyserRelease(void* analyser);


/**
* Change the default state(ON) of blink detector
*/
ANALYSER_API void setAnalyserBlinkFusionState(void* analyser, bool blinkFusionState);

/**
 * Change the default state(ON) of tongue detector
 */
ANALYSER_API void setAnalyserTongueFusionState(void* analyser, bool tongueFusionState);

/**
 * Change the default state(ON) of eye gaze detector
 */
ANALYSER_API void setAnalyserGazeFusionState(void* analyser, bool gazeFusionState);

/**
 * Update face image and 2D/3D landmarks to solve expressions
 *
 * Input color image's data of format BGR.If the input source is 2D image and 2D landmarks,
 * landmarkdDepth should be empty. The expressions solved with 3D landmarks are not good as
 * which solved with 2D landmarks.
 *
 * @param analyser pointer to analyser instance
 * @param rows BGR image rows
 * @param cols BGR image cols
 * @param data pointer to image data array, order is b-g-r-b-g-r..., row major
 * @param landmarks2d coordinates of 2D landmarks with format x1,x2,...xn,y1,y2...yn
 * @param length2d length of landmarks2d array, e.g. 136, 148
 * @param landmarks3d coordinates of 3D landmarks, which can be empty
 * @param length3d length of landmark3d
 *
 * @return 0 if success
 */
ANALYSER_API int analyserUpdate(
	void* analyser,
	int rows, int cols, unsigned char* data,
	const float* landmarks2d, int length2d,
	const float* landmarks3d = 0, int length3d = 0
);

/** 
 * Estimate face shape with landmarks
 *
 * @param analyser pointer to analyser instance
 * @param landmarks2d coordinates of 2d landmarks with format x1, x2, ..., y1, y2, ...
 * @param length2d length of landmarks2d array
 * @param landmarks3d coordinates of 3D landmarks, which can be empty
 * @param length3d length of landmark3d 
 *
 * @return 0 if success
 */
ANALYSER_API int analyserEstimateShape(
	void* analyser,
	const float* landmarks2d, int length2d,
	const float* landmarks3d = 0, int length3d = 0);


/**
 * Get analyser return result in struct AnalyserResult
 *
 * @param analyser Pointer to analyser instance
 * @param result Pointer to struct AnalyserResult instance
 */
ANALYSER_API void getAnalyserResult(void* analyser, AnalyserResult* result);

/**
 * Get 52 expression coefficients in the range [0,1]
 * 
 * @param analyser Pointer to analyser instance
 * @param expressions Array allocated to save expressions
 * @param length Input array length which SHOULD be larger than 52
 */
ANALYSER_API void getAnalyserExpressions(void* analyser, float* expressions, int length);

/**
* Get Euler angles of head rotation
*
* @param analyser Pointer to analyser instance
* @param pose Array allocated to save rotation
* @param length Input array length which SHOULD be larger than 3
*/
ANALYSER_API void getAnalyserRotation(void* analyser, float* pose, int length);

/**
* Get 50 shape coefficients in the range [0,1]
*
* @param analyser Pointer to analyser instance
* @param shape Array allocated to save shape
* @param length Input array length which SHOULD be larger than 50
*/
ANALYSER_API void getAnalyserShape(void* analyser, float* shape, int length);

/**
* Get 7 dimension head pose not smoothed
*
* @param analyser Pointer to analyser instance
* @param headpose Array allocated to save head pose
* @param length Input array length which SHOULD be larger than 7
*/
ANALYSER_API void getAnalyserHeadPose(void* analyser, float* headpose, int length);

/**
* Get 6 dimension tongue status
*
* 0-up, 1-front, 2-roll up, 3-roll down, 4-roll left, 5-roll right.
* Status indices 1,4,5 are set by results of neural network, others are
* set according to tongue blendshapes of avatar.
*
* @param analyser Pointer to analyser instance
* @param tongue Array allocated to save tongue states
* @param length Input array length which SHOULD be larger than 6
*/
ANALYSER_API void getAnalyserTongueResults(void* analyser, float* tongue, int length);

/**
* Get pitch and yaw degree of eye gaze
*
* @param analyser Pointer to analyser instance
* @param gazeDegree Array allocated to save eye gaze degree
* @param length Input array length which SHOULD be larger than 2
*/
ANALYSER_API void getAnalyserGazeDegree(void* analyser, float* gazeDegree, int length);

/**
* Get eye gaze direction which is unit vector
*
* @param analyser Pointer to analyser instance
* @param gazeDirection Array allocated to save eye gaze direction
* @param length Input array length which SHOULD be larger than 3
*/
ANALYSER_API void getAnalyserGazeDirection(void* analyser, float* gazeDirection, int length);

#ifdef __cplusplus
}
#endif

#endif 
