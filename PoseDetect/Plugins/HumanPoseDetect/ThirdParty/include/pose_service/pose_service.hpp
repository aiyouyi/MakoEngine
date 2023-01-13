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
	* SDK ��ʼ�������ڴ���sdk�������Դ
	*
	* @note ����ڵ��������ӿ�֮ǰ���øó�ʼ���ӿ�
	*/
	POSE_SERVICE_API void POSEInit();

	/**
	* ʶ���������̬����������
	*
	* @note ���øýӿ�ǰ�����ȵ��� POSEInit() ��ʼ����Դ;
	*       �������� "�Ƿ����ֲ�"�����ã����ڵ��øýӿ�ǰ������ϣ�
	*
	* @param ptr������ͼ���ָ��
	* @param height��ͼ��ĸ�
	* @param width��ͼ��Ŀ�
	* @param format��ͼ������ݸ�ʽ����bgr���͵�ͼ��ʹ�� SDKFormat::SDK_BGR
	* @param mesh_ptr���������ݵ�ָ�룬���ڴ洢ʶ��Ľ��
	*/
	POSE_SERVICE_API void POSEProcess(unsigned char* ptr, int height, int width, SDKFormat format, float* mesh_ptr);
	
	/**
	* �����Ƿ����ֲ����粻���øýӿڣ�Ĭ�ϲ�����ֲ�
	*
	* @note ���øýӿ�ǰ�����ȵ��� POSEInit() ��ʼ����Դ��
	*      
	* @param detect_hand������Ϊtrue���� SDK �����ֲ���Ϣ�����أ�
	*/
	POSE_SERVICE_API void POSESetHandStatus(bool detect_hand);
	
	/**
	* ����������̬��������ʱ���˲����Ĵ�С���粻���øýӿڣ������˲�����СĬ������Ϊ5��
	*
	* @note ���øýӿ�ǰ�����ȵ��� POSEInit() ��ʼ����Դ��
	*
	* @param kernal_ptr��ָ���˲����˴�С������ָ�룬������������52��Ԫ�أ�
	*/
	POSE_SERVICE_API void POSESetFilterKernal(const int* kernal_ptr);

	/**
	* ���� SDK ����ģ���ļ����ڵ�·�����粻���øýӿڣ�·��Ĭ������Ϊ��Ŀ��Ŀ¼
	*
	* @note ���øýӿ�ǰ�����ȵ��� POSEInit() ��ʼ����Դ��
	*
	* @param path��ָ��·���ַ�����ָ�룻
	*/
	POSE_SERVICE_API void POSESetModelsPath(const char* path);

	/**
	* SDK ��������ǰ��ʽ�ͷ���Դ�ļ����ýӿڿ��Բ����ã���Դ���Զ��ͷţ�
	*
	*/
	POSE_SERVICE_API void POSEClose();

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // POSE_SERVICE_HPP
