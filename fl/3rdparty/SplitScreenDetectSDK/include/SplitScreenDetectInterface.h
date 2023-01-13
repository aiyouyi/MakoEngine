#ifndef SPLIT_SCREEN_DETECT_INTERFACE_H
#define SPLIT_SCREEN_DETECT_INTERFACE_H

#include "SplitScreenDetectDef.h"


//---------------------------------------------------------------------------------------
/**Detect 分屏检测
 * @param pBuffer image data
 * @param width image's height
 * @param height image's width
 * @param stride image's row pixels bytes
 * @param ctType image's format, BGRA/RGBA/BGR/RGB
*/
SPLIT_SCREEN_DETECT_EXPORTS_API
SSD_SCREEN_TYPE ccScreenSplitDetect(unsigned char* pBuffer, int width, int height, int stride, int ctType);

#endif //SPLIT_SCREEN_DETECT_INTERFACE_H