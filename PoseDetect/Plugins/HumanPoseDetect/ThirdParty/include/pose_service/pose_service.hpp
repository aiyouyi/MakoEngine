#ifndef POSE_SERVICE_HPP
#define POSE_SERVICE_HPP

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#define POSE_SERVICE_API __declspec(dllexport)
	// use SDK prefix to avoid name confliction in iOS
	typedef enum {
		SDK_BGR,
		SDK_RGB,
		SDK_I420,
		SDK_NV12,
		SDK_NV21
	} SDKFormat;

	// use SDK prefix to avoid name confliction
	
	/**
	* SDK 初始化，用于创建sdk所需的资源
	*
	* @note 务必在调用其他接口之前调用该初始化接口
	*/
	POSE_SERVICE_API void POSEInit();

	/**
	* 识别人体的姿态参数并返回
	*
	* @note 调用该接口前，请先调用 POSEInit() 初始化资源;
	*       如需设置 "是否检测手部"等配置，请在调用该接口前设置完毕；
	*
	* @param ptr，输入图像的指针
	* @param height，图像的高
	* @param width，图像的宽
	* @param format，图像的数据格式，如bgr类型的图像使用 SDKFormat::SDK_BGR
	* @param mesh_ptr，返回数据的指针，用于存储识别的结果
	*/
	POSE_SERVICE_API void POSEProcess(unsigned char* ptr, int height, int width, SDKFormat format, float* mesh_ptr);
	
	/**
	* 设置是否检测手部，如不调用该接口，默认不检测手部
	*
	* @note 调用该接口前，请先调用 POSEInit() 初始化资源；
	*      
	* @param detect_hand，设置为true，则 SDK 会检测手部信息并返回；
	*/
	POSE_SERVICE_API void POSESetHandStatus(bool detect_hand);
	
	/**
	* 设置人体姿态参数后处理时的滤波器的大小，如不调用该接口，所有滤波器大小默认设置为5；
	*
	* @note 调用该接口前，请先调用 POSEInit() 初始化资源；
	*
	* @param kernal_ptr，指向滤波器核大小的数组指针，该数组至少有52个元素；
	*/
	POSE_SERVICE_API void POSESetFilterKernal(const int* kernal_ptr);

	/**
	* 设置 SDK 所需模型文件所在的路径，如不调用该接口，路径默认设置为项目根目录
	*
	* @note 调用该接口前，请先调用 POSEInit() 初始化资源；
	*
	* @param path，指向路径字符串的指针；
	*/
	POSE_SERVICE_API void POSESetModelsPath(const char* path);

	/**
	* SDK 结束调用前显式释放资源文件，该接口可以不调用，资源会自动释放；
	*
	*/
	POSE_SERVICE_API void POSEClose();

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // POSE_SERVICE_HPP
