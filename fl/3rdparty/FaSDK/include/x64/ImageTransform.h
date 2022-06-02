#pragma once
#include "FaceDetectorDef.h"

FACEDETECTOR_EXPORTS_API unsigned char *Transform(unsigned char *pSrcBuffer, int width, int height, int rowBytes, int ctSrcType, unsigned char *pDestBuffer, int ctDestType);

FACEDETECTOR_EXPORTS_API unsigned char *TransformToGrey(unsigned char *pSrcBuffer, int width, int height, int rowBytes, int ctSrcType, unsigned char *pDestBuffer);
