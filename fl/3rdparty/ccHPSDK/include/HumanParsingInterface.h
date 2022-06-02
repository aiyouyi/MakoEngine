#ifndef HUMAN_PARSING_INTERFACE_H
#define HUMAN_PARSING_INTERFACE_H

#include "HumanParsingDef.h"

// ======== SDK相关接口 ========
// 设置输入图像大小，支持任意尺寸；此接口须在具体算法模块的Activate接口之前设置，若输入图像尺寸改变，则需重新设置
HUMAN_PARSING_EXPORTS_API void ccHPSetInputSize(int width, int height);
// 设置推理线程数，默认为1
HUMAN_PARSING_EXPORTS_API void ccHPSetThreadNum(int threadNum);
// 执行当前帧Parsing（图像数据所在内存位置，图像宽，图像高，图像单行像素所占字节数，图像格式BGR、RGB等），各算法模块会根据开启与否，依次执行分割。
HUMAN_PARSING_EXPORTS_API bool ccHPParsing(unsigned char* pImgData, int width, int height, int stride, int ctType);
// 重置内部缓存，在改变视频输入流时须Reset
HUMAN_PARSING_EXPORTS_API void ccHPReset();

// ======== 背景分割相关接口 ========
// 启用背景分割
HUMAN_PARSING_EXPORTS_API bool ccHPActivateBackgroundSeg(const char* szModelPathBig, const char* szModelPathSmall);
// 关闭背景分割
HUMAN_PARSING_EXPORTS_API void ccHPDeactivateBackgroundSeg();
// 查询背景分割输出Mask尺寸，Mask的尺寸并不与输入图像尺寸一致，需自行再做resize处理。当SDK的输入图像尺寸发生改变时，输出Mask尺寸也会改变。OutChannel固定为1
HUMAN_PARSING_EXPORTS_API void ccHPGetBackgroundMaskOutShape(int* pOutWidth, int* pOutHeight, int* pOutChannel);
// 获取背景分割Mask，值为0~255，0代表背景，255代表人。Mask的内存需调用者自行申请，且size要与Mask的输出尺寸匹配
HUMAN_PARSING_EXPORTS_API bool ccHPGetBackgroundMask(unsigned char* pArrSegMask, int size);

// ======== 头发分割相关接口 ========
// 启用头发分割
HUMAN_PARSING_EXPORTS_API bool ccHPActivateHairSeg(const char* szModelPath);
// 关闭头发分割
HUMAN_PARSING_EXPORTS_API void ccHPDeactivateHairSeg();
// 查询头发分割输出Mask尺寸，Mask的尺寸并不与输入图像尺寸一致，需自行再做resize处理。当SDK的输入图像尺寸发生改变时，输出Mask尺寸也会改变。OutChannel固定为1
HUMAN_PARSING_EXPORTS_API void ccHPGetHairMaskOutShape(int* pOutWidth, int* pOutHeight, int* pOutChannel);
// 获取头发分割Mask，值为0~255，0代表背景，255代表头发。Mask的内存需调用者自行申请，且size要与Mask的输出尺寸匹配
HUMAN_PARSING_EXPORTS_API bool ccHPGetHairMask(unsigned char* pArrSegMask, int size);

#endif // HUMAN_PARSING_INTERFACE_H