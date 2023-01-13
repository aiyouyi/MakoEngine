#ifndef FITERBASE_H
#define FITERBASE_H

#include"CCProgram.h"
#include "DX11ImageFilterDef.h"

class DX11IMAGEFILTER_EXPORTS_CLASS FilterBase
{
public:
    FilterBase();
    ~FilterBase();

    void FilterToTexture(GLuint inputTex,int nWidth,int nHeight);

    void FilterToTexture(GLuint inputTex, int nWidth, int nHeight, float *pVertex, float *pTexCoord);

    void FilterToGray(GLuint inputTex,int nWidth,int nHeight);

private:

    CCProgram *m_program;
    CCProgram *m_programGray;
};

#endif // FITERBASE_H
