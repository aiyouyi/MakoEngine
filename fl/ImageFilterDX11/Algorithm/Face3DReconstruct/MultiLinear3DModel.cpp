#include "MultiLinear3DModel.h"
#include <stdlib.h>
#include <vector>
#include <thread>
#include "BaseDefine/Define.h"
#if defined(__arm__)||defined(__arm64__) || defined(__aarch64__)
#include "SSE2NEON.h"
#else
#include <emmintrin.h>
#endif
namespace mt3dface
{

    struct MT3DDataCore
    {
        float *pMat;
        float *pCore;
        short *pCoreSrc;
		float *pCoreCore;
        int nPoint;
        int nFace;    
    };
    
    static void* aligned_malloc(size_t size, size_t alignment)
    {
        //#if !defined(__arm__) && !defined(__arm64__) && !defined(__aarch64__)
        //    return _mm_malloc(size, alignment);
        //#else
        if (alignment & (alignment - 1))
        {
            return NULL;
        }
        else
        {
            void *praw = malloc(sizeof(void*) + size + alignment);
            if (praw)
            {
                void *pbuf = reinterpret_cast<void*>(reinterpret_cast<size_t>(praw)+sizeof(void*));
                void *palignedbuf = reinterpret_cast<void*>((reinterpret_cast<size_t>(pbuf) | (alignment - 1)) + 1);
                (static_cast<void**>(palignedbuf)[-1] = praw);
                return palignedbuf;
            }
            else
            {
                return NULL;
            }
        }
        //#endif
    }
    
    static void aligned_free(void *palignedmem)
    {
        //#if !defined(__arm__) && !defined(__arm64__) && !defined(__aarch64__)
        //    _mm_free(palignedmem);
        //#else
        free(reinterpret_cast<void*>((static_cast<void**>(palignedmem)[-1])));
        //SAFE_DELETE_ARRAY(palignedmem);
        //#endif
    }
    
    static void* AllocMemory(unsigned int Size, bool ZeroMemory)
    {
        void *Ptr = aligned_malloc(Size, 32);
        if (Ptr != NULL && ZeroMemory == true)
            memset(Ptr, 0, Size);
        return Ptr;
    }
    
    static void FreeMemory(void *Ptr)
    {
        if (Ptr != NULL) aligned_free(Ptr);
    }
    
    void *UnzipData(void *args)
    {
        float fTempData[MLM_EXPRESS_DIMS*MLM_IDENTITY_DIMS * 3] = {0};
        MT3DDataCore *m_DataCore = (MT3DDataCore*)args;
        float *pMat = m_DataCore->pMat;
        float *pCore = m_DataCore->pCore;
        short *pCoreSrc = m_DataCore->pCoreSrc;
        float *pCoreCore = m_DataCore->pCoreCore;
        int nPoint = m_DataCore->nPoint;
        int nFace = m_DataCore->nFace;
        
        for (int k = 0; k < nPoint; k++)
        {
            float *pMatK = pMat + k * 300;
            float *pCorek = pCore;
            {
                float *pMatKx = pMatK;
                float *pMatKy = pMatK + 100;
                float *pMatKz = pMatK + 200;
                float x[4] = { 0 }, y[4] = { 0 }, z[4] = { 0 };
                __m128* src1 = (__m128*)pMatKx;
                __m128* src2 = (__m128*)pMatKy;
                __m128* src3 = (__m128*)pMatKz;
                __m128* dst = (__m128*)pCorek;
                __m128 m1, m2, m3;
                m1 = _mm_setzero_ps();
                m2 = _mm_setzero_ps();
                m3 = _mm_setzero_ps();
                for (int j = 0; j < 100; j += 4, src1++, src2++, src3++, dst++)
                {
                    m1 = _mm_add_ps(m1, _mm_mul_ps(*src1, *dst));
                    m2 = _mm_add_ps(m2, _mm_mul_ps(*src2, *dst));
                    m3 = _mm_add_ps(m3, _mm_mul_ps(*src3, *dst));
                }
                pCorek += 100;
                _mm_storeu_ps(x, m1);
                _mm_storeu_ps(y, m2);
                _mm_storeu_ps(z, m3);
                *pCoreCore++ = x[0] + x[1] + x[2] + x[3];
                *pCoreCore++ = y[0] + y[1] + y[2] + y[3];
                *pCoreCore++ = z[0] + z[1] + z[2] + z[3];
                
                *pCoreSrc++ = 0;
                *pCoreSrc++ = 0;
                *pCoreSrc++ = 0;
            }
            
            float* pTemp = fTempData;
            for (int i = 1; i < nFace; i++)
            {
                float *pMatKx = pMatK;
                float *pMatKy = pMatK + 100;
                float *pMatKz = pMatK + 200;
                float x[4] = { 0 }, y[4] = { 0 }, z[4] = { 0 };
                __m128* src1 = (__m128*)pMatKx;
                __m128* src2 = (__m128*)pMatKy;
                __m128* src3 = (__m128*)pMatKz;
                __m128* dst = (__m128*)pCorek;
                __m128 m1, m2, m3;
                m1 = _mm_setzero_ps();
                m2 = _mm_setzero_ps();
                m3 = _mm_setzero_ps();

                {
                    m1 = _mm_add_ps(m1, _mm_mul_ps(*src1++, *dst));
                    m2 = _mm_add_ps(m2, _mm_mul_ps(*src2++, *dst));
                    m3 = _mm_add_ps(m3, _mm_mul_ps(*src3++, *dst++));
                    m1 = _mm_add_ps(m1, _mm_mul_ps(*src1++, *dst));
                    m2 = _mm_add_ps(m2, _mm_mul_ps(*src2++, *dst));
                    m3 = _mm_add_ps(m3, _mm_mul_ps(*src3++, *dst++));
                    m1 = _mm_add_ps(m1, _mm_mul_ps(*src1++, *dst));
                    m2 = _mm_add_ps(m2, _mm_mul_ps(*src2++, *dst));
                    m3 = _mm_add_ps(m3, _mm_mul_ps(*src3++, *dst++));
                    m1 = _mm_add_ps(m1, _mm_mul_ps(*src1++, *dst));
                    m2 = _mm_add_ps(m2, _mm_mul_ps(*src2++, *dst));
                    m3 = _mm_add_ps(m3, _mm_mul_ps(*src3++, *dst++));
                    m1 = _mm_add_ps(m1, _mm_mul_ps(*src1++, *dst));
                    m2 = _mm_add_ps(m2, _mm_mul_ps(*src2++, *dst));
                    m3 = _mm_add_ps(m3, _mm_mul_ps(*src3++, *dst++));
                    m1 = _mm_add_ps(m1, _mm_mul_ps(*src1++, *dst));
                    m2 = _mm_add_ps(m2, _mm_mul_ps(*src2++, *dst));
                    m3 = _mm_add_ps(m3, _mm_mul_ps(*src3++, *dst++));
                    m1 = _mm_add_ps(m1, _mm_mul_ps(*src1++, *dst));
                    m2 = _mm_add_ps(m2, _mm_mul_ps(*src2++, *dst));
                    m3 = _mm_add_ps(m3, _mm_mul_ps(*src3++, *dst++));
                    m1 = _mm_add_ps(m1, _mm_mul_ps(*src1++, *dst));
                    m2 = _mm_add_ps(m2, _mm_mul_ps(*src2++, *dst));
                    m3 = _mm_add_ps(m3, _mm_mul_ps(*src3++, *dst++));
                    m1 = _mm_add_ps(m1, _mm_mul_ps(*src1++, *dst));
                    m2 = _mm_add_ps(m2, _mm_mul_ps(*src2++, *dst));
                    m3 = _mm_add_ps(m3, _mm_mul_ps(*src3++, *dst++));
                    m1 = _mm_add_ps(m1, _mm_mul_ps(*src1++, *dst));
                    m2 = _mm_add_ps(m2, _mm_mul_ps(*src2++, *dst));
                    m3 = _mm_add_ps(m3, _mm_mul_ps(*src3++, *dst++));
                    m1 = _mm_add_ps(m1, _mm_mul_ps(*src1++, *dst));
                    m2 = _mm_add_ps(m2, _mm_mul_ps(*src2++, *dst));
                    m3 = _mm_add_ps(m3, _mm_mul_ps(*src3++, *dst++));
                    m1 = _mm_add_ps(m1, _mm_mul_ps(*src1++, *dst));
                    m2 = _mm_add_ps(m2, _mm_mul_ps(*src2++, *dst));
                    m3 = _mm_add_ps(m3, _mm_mul_ps(*src3++, *dst++));
                    m1 = _mm_add_ps(m1, _mm_mul_ps(*src1++, *dst));
                    m2 = _mm_add_ps(m2, _mm_mul_ps(*src2++, *dst));
                    m3 = _mm_add_ps(m3, _mm_mul_ps(*src3++, *dst++));
                    m1 = _mm_add_ps(m1, _mm_mul_ps(*src1++, *dst));
                    m2 = _mm_add_ps(m2, _mm_mul_ps(*src2++, *dst));
                    m3 = _mm_add_ps(m3, _mm_mul_ps(*src3++, *dst++));
                    m1 = _mm_add_ps(m1, _mm_mul_ps(*src1++, *dst));
                    m2 = _mm_add_ps(m2, _mm_mul_ps(*src2++, *dst));
                    m3 = _mm_add_ps(m3, _mm_mul_ps(*src3++, *dst++));
                    m1 = _mm_add_ps(m1, _mm_mul_ps(*src1++, *dst));
                    m2 = _mm_add_ps(m2, _mm_mul_ps(*src2++, *dst));
                    m3 = _mm_add_ps(m3, _mm_mul_ps(*src3++, *dst++));
                    m1 = _mm_add_ps(m1, _mm_mul_ps(*src1++, *dst));
                    m2 = _mm_add_ps(m2, _mm_mul_ps(*src2++, *dst));
                    m3 = _mm_add_ps(m3, _mm_mul_ps(*src3++, *dst++));
                    m1 = _mm_add_ps(m1, _mm_mul_ps(*src1++, *dst));
                    m2 = _mm_add_ps(m2, _mm_mul_ps(*src2++, *dst));
                    m3 = _mm_add_ps(m3, _mm_mul_ps(*src3++, *dst++));
                    m1 = _mm_add_ps(m1, _mm_mul_ps(*src1++, *dst));
                    m2 = _mm_add_ps(m2, _mm_mul_ps(*src2++, *dst));
                    m3 = _mm_add_ps(m3, _mm_mul_ps(*src3++, *dst++));
                    m1 = _mm_add_ps(m1, _mm_mul_ps(*src1++, *dst));
                    m2 = _mm_add_ps(m2, _mm_mul_ps(*src2++, *dst));
                    m3 = _mm_add_ps(m3, _mm_mul_ps(*src3++, *dst++));
                    m1 = _mm_add_ps(m1, _mm_mul_ps(*src1++, *dst));
                    m2 = _mm_add_ps(m2, _mm_mul_ps(*src2++, *dst));
                    m3 = _mm_add_ps(m3, _mm_mul_ps(*src3++, *dst++));
                    m1 = _mm_add_ps(m1, _mm_mul_ps(*src1++, *dst));
                    m2 = _mm_add_ps(m2, _mm_mul_ps(*src2++, *dst));
                    m3 = _mm_add_ps(m3, _mm_mul_ps(*src3++, *dst++));
                    m1 = _mm_add_ps(m1, _mm_mul_ps(*src1++, *dst));
                    m2 = _mm_add_ps(m2, _mm_mul_ps(*src2++, *dst));
                    m3 = _mm_add_ps(m3, _mm_mul_ps(*src3++, *dst++));
                    m1 = _mm_add_ps(m1, _mm_mul_ps(*src1++, *dst));
                    m2 = _mm_add_ps(m2, _mm_mul_ps(*src2++, *dst));
                    m3 = _mm_add_ps(m3, _mm_mul_ps(*src3++, *dst++));
                    m1 = _mm_add_ps(m1, _mm_mul_ps(*src1++, *dst));
                    m2 = _mm_add_ps(m2, _mm_mul_ps(*src2++, *dst));
                    m3 = _mm_add_ps(m3, _mm_mul_ps(*src3++, *dst++));
                }
                
                pCorek += 100;

                _mm_storeu_ps(x, m1);
                _mm_storeu_ps(y, m2);
                _mm_storeu_ps(z, m3);
                
                *pTemp++ = x[0] + x[1] + x[2] + x[3];
                *pTemp++ = y[0] + y[1] + y[2] + y[3];
                *pTemp++ = z[0] + z[1] + z[2] + z[3];
            }
            
            
             __m128 addDst = _mm_set_ps1(MLM_FLOAT_TO_SHORT);
            int nBlockSize = 8, nBlock = ((MLM_EXPRESS_DIMS*MLM_IDENTITY_DIMS - 1) * 3) / nBlockSize;
            int nSize = (MLM_EXPRESS_DIMS*MLM_IDENTITY_DIMS - 1)  * 3;
            int nTotalSize = nBlock * nBlockSize;
            
            // for(int i = 0; i < nTotalSize; i += nBlockSize)
            // {
            //     __m128 T1 = _mm_mul_ps(_mm_loadu_ps(fTempData + i + 0), addDst);
            //     __m128 T2 = _mm_mul_ps(_mm_loadu_ps(fTempData + i + 4), addDst);
            //     __m128i T = _mm_packs_epi32(_mm_cvttps_epi32(T1), _mm_cvttps_epi32(T2));
            //     _mm_storeu_si128((__m128i *)(pCoreSrc + i), T);
            // }
            
            for(int i = 0; i < nSize; i++)
            {
                pCoreSrc[i] = fTempData[i] * MLM_FLOAT_TO_SHORT;
            }
            
            pCoreSrc += nSize;
        }
        return ((void*)100);
    }


MultiLinear3DModel::MultiLinear3DModel()
{
	InitData();
}


MultiLinear3DModel::~MultiLinear3DModel()
{
	ClearData();
}



void MultiLinear3DModel::InitData()
{
	m_pCore = NULL;
	m_p3DFaceVertex = new float[MLM_FACE_3D_UV_VERTEX * 3];

	m_pTriangleIndex = new short3D[3 * MLM_TRIANGLE_WITH_LIPS];
	m_pTextureCoordinates = new float[2 * MLM_FACE_3D_UV_VERTEX];
    m_pFaceTriangleIndex = new short3D[3 * MLM_FACE_TRIANGLE_WITH_LIPS];
    m_pFaceTextureCoordinates = new float[2 * MLM_FACE_3D_UV_VERTEX];
	m_pSFMTextureCoordinates = new float[2 * MLM_FACE_3D_UV_VERTEX];
    m_pFaceNormal = new float[3 * MLM_FACE_3D_UV_VERTEX];

	m_pContourIndex = new short3D[MLM_CONTOUR_INDEX];
	m_pContourVertex = new float[3 * MLM_CONTOUR_INDEX];


	m_pIdentityVertex = new float[MLM_LANMARK_INDEX * 3];
	m_pIdentityBasis = new float[MLM_LANMARK_INDEX * 3 * MLM_IDENTITY_DIMS];
	m_pExpressVertex = new float[MLM_LANMARK_INDEX * 3];
	m_pExpressBasis = new float[MLM_LANMARK_INDEX * 3 * MLM_EXPRESS_DIMS];

	m_pLanmarkVertex = new float[MLM_LANMARK_INDEX * 3];

	m_pBlendShape = NULL;
	m_bUseBlendShape = false;
}

void MultiLinear3DModel::ClearData()
{
	SAFE_DELETE_ARRAY(m_pCore);
	SAFE_DELETE_ARRAY(m_pBlendShape);
	SAFE_DELETE_ARRAY(m_p3DFaceVertex);
	SAFE_DELETE_ARRAY(m_pTriangleIndex);
	SAFE_DELETE_ARRAY(m_pTextureCoordinates);
    SAFE_DELETE_ARRAY(m_pFaceTriangleIndex);
    SAFE_DELETE_ARRAY(m_pFaceTextureCoordinates);
	SAFE_DELETE_ARRAY(m_pSFMTextureCoordinates);
    SAFE_DELETE_ARRAY(m_pFaceNormal);
	SAFE_DELETE_ARRAY(m_pIdentityBasis);
	SAFE_DELETE_ARRAY(m_pIdentityVertex);
	SAFE_DELETE_ARRAY(m_pExpressBasis);
	SAFE_DELETE_ARRAY(m_pExpressVertex);
	SAFE_DELETE_ARRAY(m_pContourIndex);
	SAFE_DELETE_ARRAY(m_pContourVertex);
	SAFE_DELETE_ARRAY(m_pLanmarkVertex);
	SAFE_DELETE_ARRAY(m_pCoreCore);
}



bool MultiLinear3DModel::LoadModel(char *pModelName, char *pContourName, char *pLanmarkName, char *pTempFile, char *pMatFile, bool bUseBlendShape)
    {
        LoadContourLine(pContourName);
        LoadLanmarkIndex(pLanmarkName);
        Load3DObj(pTempFile);
        LoadExpressMat(pMatFile);
        
        FILE *pFileRead = fopen(pModelName, "rb");
        if (pFileRead == NULL)
        {
            return false;
        }
        m_bUseBlendShape = bUseBlendShape;
        if (bUseBlendShape)
        {
            m_pBlendShape = new float[MLM_EXPRESS_DIMS*MLM_FACE_3D_VERTEX * 3];
            fread(m_pBlendShape, sizeof(float) *MLM_EXPRESS_DIMS*MLM_FACE_3D_VERTEX * 3, 1, pFileRead);
            fclose(pFileRead);
            return true;
        }
        
        SAFE_DELETE_ARRAY(m_pCore);
        m_pCore = new short[MLM_IDENTITY_DIMS*MLM_EXPRESS_DIMS*MLM_FACE_3D_VERTEX * 3];
        m_pCoreCore = new float[MLM_FACE_3D_VERTEX * 3];
        float* pMat = (float*)AllocMemory(sizeof(float)*MLM_FACE_3D_VERTEX * 3 * 100, false);
        float *pCore = (float*)AllocMemory(sizeof(float) * 100 * MLM_IDENTITY_DIMS*MLM_EXPRESS_DIMS, false);
        
        float fMatMin = -0.061176262795925;
        float fMatMax = 0.057269629091024;
        unsigned short* pMatCopy = (unsigned short*)AllocMemory(sizeof(unsigned short)*MLM_FACE_3D_VERTEX * 3 * 100, false);
        
        fread(pMatCopy, sizeof(unsigned short) * 100 * MLM_FACE_3D_VERTEX * 3, 1, pFileRead);
        fread(pCore, sizeof(float) * 100 * MLM_IDENTITY_DIMS*MLM_EXPRESS_DIMS, 1, pFileRead);
        fclose(pFileRead);
        
        int nMatSize = MLM_FACE_3D_VERTEX * 3 * 100;

        float fDiff = (fMatMax - fMatMin)/65536;
        for(int i = 0; i < nMatSize; i++)
        {
             pMat[i] = pMatCopy[i] * fDiff + fMatMin;
        }
        
        FreeMemory(pMatCopy);

        
        int nFace = MLM_EXPRESS_DIMS*MLM_IDENTITY_DIMS;
        short *pCoreSrc = m_pCore;
        int ThreadNum = 3;
        std::vector<std::thread> threads;
        threads.resize(ThreadNum);
        MT3DDataCore *DataCore = new MT3DDataCore[ThreadNum];
        int nPoint = MLM_FACE_3D_VERTEX / ThreadNum;
        for (int i = 0; i < ThreadNum; i++)
        {
            DataCore[i].pMat = pMat + i * 300 * nPoint;
            DataCore[i].pCore = pCore;
            DataCore[i].nFace = nFace;
            DataCore[i].nPoint = nPoint;
            DataCore[i].pCoreSrc = pCoreSrc + nPoint * 3 * i*nFace;
            DataCore[i].pCoreCore = m_pCoreCore + nPoint *i * 3;
        }
        
        for (int i = 0; i < ThreadNum; ++i)
        {
            threads[i] = std::thread(UnzipData, (void*)(DataCore + i));
        }
        for (int i = 0; i < ThreadNum; i++)
        {
            threads[i].join();
        }
        
        delete[]DataCore;
        
        FreeMemory(pMat);
        FreeMemory(pCore);
        return true;
    }

bool MultiLinear3DModel::LoadModelFromData(unsigned char *pModelData, unsigned char *pContourData, unsigned char *pLanmarkData, unsigned char *pTempData, unsigned char *pMatDta)
{
	if (pModelData == NULL || pContourData == NULL || pLanmarkData == NULL || pTempData == NULL || pMatDta == NULL)
	{
		return false;
	}

	memcpy(m_pContourLine, pContourData ,sizeof(short3D)*MLM_CONTOUR_LINE);
	memcpy(m_pContourIndex, pContourData + sizeof(short3D)*MLM_CONTOUR_LINE,sizeof(short3D)*MLM_CONTOUR_INDEX);

	memcpy(m_p2Dindex, pLanmarkData, sizeof(short3D)*MLM_LANMARK_INDEX);
	memcpy(m_p3Dindex, pLanmarkData + sizeof(short3D)*MLM_LANMARK_INDEX, sizeof(short3D)*MLM_LANMARK_INDEX);


	memcpy(m_pUVMap, pTempData, sizeof(short3D) * (MLM_FACE_3D_UV_VERTEX - MLM_FACE_3D_VERTEX));
	int offest = sizeof(short3D) * (MLM_FACE_3D_UV_VERTEX - MLM_FACE_3D_VERTEX);
	memcpy(m_pTextureCoordinates, pTempData + offest, sizeof(float) * 2 * MLM_FACE_3D_UV_VERTEX);
	offest += sizeof(float) * 2 * MLM_FACE_3D_UV_VERTEX;
	memcpy(m_pTriangleIndex, pTempData + offest, MLM_TRIANGLE_WITH_LIPS * 3 * sizeof(short3D));
	offest += MLM_TRIANGLE_WITH_LIPS * 3 * sizeof(short3D);
	memcpy(m_pFaceTextureCoordinates, pTempData + offest, sizeof(float)*MLM_FACE_3D_UV_VERTEX * 2);
	offest += sizeof(float)*MLM_FACE_3D_UV_VERTEX * 2;
	memcpy(m_pFaceTriangleIndex, pTempData + offest, sizeof(short3D) * MLM_FACE_TRIANGLE_WITH_LIPS * 3);
	offest += sizeof(short3D) * MLM_FACE_TRIANGLE_WITH_LIPS * 3;
	memcpy(m_pFaceNormal, pTempData + offest, sizeof(float)*MLM_FACE_3D_UV_VERTEX * 3);
	offest += sizeof(float)*MLM_FACE_3D_UV_VERTEX * 3;
	memcpy(m_pSFMTextureCoordinates, pTempData + offest, sizeof(float)*MLM_FACE_3D_UV_VERTEX * 2);

	memcpy(m_pExpress25To47Mat, pMatDta, sizeof(float) * 25 * 47);
	memcpy(m_pIdentityInitParam, pMatDta+ sizeof(float) * 25 * 47, sizeof(float)*MLM_IDENTITY_DIMS);
	memcpy(m_pExpressInitParam, pMatDta+ sizeof(float) * 25 * 47+ sizeof(float)*MLM_IDENTITY_DIMS,sizeof(float) * MLM_EXPRESS_DIMS);


	m_pCore = new short[MLM_IDENTITY_DIMS*MLM_EXPRESS_DIMS*MLM_FACE_3D_VERTEX * 3];
	m_pCoreCore = new float[MLM_FACE_3D_VERTEX * 3];	
        
    float fMatMin = -0.061176262795925;
    float fMatMax = 0.057269629091024;
    unsigned short* pMatCopy = (unsigned short*)AllocMemory(sizeof(unsigned short)*MLM_FACE_3D_VERTEX * 3 * 100, false);

	float* pMat = (float*)AllocMemory(sizeof(float)*MLM_FACE_3D_VERTEX * 3 * 100, false);
	float *pCore = (float*)AllocMemory(sizeof(float) * 100 * MLM_IDENTITY_DIMS*MLM_EXPRESS_DIMS, false);

	memcpy(pMatCopy, pModelData, sizeof(unsigned short) * 100 * MLM_FACE_3D_VERTEX * 3);
	memcpy(pCore, pModelData+ sizeof(unsigned short) * 100 * MLM_FACE_3D_VERTEX * 3, sizeof(float) * 100 * MLM_IDENTITY_DIMS*MLM_EXPRESS_DIMS);

    int nMatSize = MLM_FACE_3D_VERTEX * 3 * 100;
        
    float fDiff = (fMatMax - fMatMin)/65536;
    for(int i = 0; i < nMatSize; i++)
    {
        pMat[i] = pMatCopy[i] * fDiff + fMatMin;
    }
        
    FreeMemory(pMatCopy);

    int nFace = MLM_EXPRESS_DIMS*MLM_IDENTITY_DIMS;
    short *pCoreSrc = m_pCore;
    int ThreadNum = 3;
    std::vector<std::thread> threads;
    threads.resize(ThreadNum);
    MT3DDataCore *DataCore = new MT3DDataCore[ThreadNum];
    int nPoint = MLM_FACE_3D_VERTEX / ThreadNum;
    for (int i = 0; i < ThreadNum; i++)
    {
        DataCore[i].pMat = pMat + i * 300 * nPoint;
        DataCore[i].pCore = pCore;
        DataCore[i].nFace = nFace;
        DataCore[i].nPoint = nPoint;
        DataCore[i].pCoreSrc = pCoreSrc + nPoint * 3 * i*nFace;
        DataCore[i].pCoreCore = m_pCoreCore + nPoint *i * 3;
    }
        
    for (int i = 0; i < ThreadNum; ++i)
    {
        threads[i] = std::thread(UnzipData, (void*)(DataCore + i));
    }
    for (int i = 0; i < ThreadNum; i++)
    {
        threads[i].join();
    }
        
    delete[]DataCore;
        
    FreeMemory(pMat);
    FreeMemory(pCore);
    return true;
}


void MultiLinear3DModel::LoadContourLine(char *pFileName)
{
	FILE *pFileRead = fopen(pFileName, "rb");
	if (pFileRead == NULL)
	{
		return;
	}
	fread(m_pContourLine, sizeof(short3D)*MLM_CONTOUR_LINE, 1, pFileRead);
	fread(m_pContourIndex, sizeof(short3D)*MLM_CONTOUR_INDEX, 1, pFileRead);
	fclose(pFileRead);
}

void MultiLinear3DModel::LoadLanmarkIndex(char *pFileName)
{
	FILE *pFileRead = fopen(pFileName, "rb");
	if (pFileRead == NULL)
	{
		return;
	}
	fread(m_p2Dindex, sizeof(short3D)*MLM_LANMARK_INDEX, 1, pFileRead);
	fread(m_p3Dindex, sizeof(short3D)*MLM_LANMARK_INDEX, 1, pFileRead);
	fclose(pFileRead);
}

void MultiLinear3DModel::Load3DObj(char *pFileName)
{
	FILE *pFileRead = fopen(pFileName, "rb");
	if (pFileRead == NULL)
	{
		return;
	}
	fread(m_pUVMap, sizeof(short3D) * (MLM_FACE_3D_UV_VERTEX - MLM_FACE_3D_VERTEX), 1, pFileRead);
	fread(m_pTextureCoordinates, sizeof(float) * 2 * MLM_FACE_3D_UV_VERTEX, 1, pFileRead);
	fread(m_pTriangleIndex, MLM_TRIANGLE_WITH_LIPS * 3 * sizeof(short3D), 1, pFileRead);
    fread(m_pFaceTextureCoordinates, sizeof(float)*MLM_FACE_3D_UV_VERTEX * 2, 1, pFileRead);
    fread(m_pFaceTriangleIndex, sizeof(short3D) * MLM_FACE_TRIANGLE_WITH_LIPS * 3, 1, pFileRead);
    fread(m_pFaceNormal, sizeof(float)*MLM_FACE_3D_UV_VERTEX * 3, 1, pFileRead);
	fread(m_pSFMTextureCoordinates, sizeof(float)*MLM_FACE_3D_UV_VERTEX * 2, 1, pFileRead);
	fclose(pFileRead);
}

void MultiLinear3DModel::LoadExpressMat(char*pFileName)
{
	FILE *pFileRead = fopen(pFileName, "rb");
	if (pFileRead == NULL)
	{
		return;
	}
	fread(m_pExpress25To47Mat, sizeof(float) * 25 * 47, 1, pFileRead);
	fread(m_pIdentityInitParam, sizeof(float)*MLM_IDENTITY_DIMS, 1, pFileRead);
	fread(m_pExpressInitParam, sizeof(float) * MLM_EXPRESS_DIMS, 1, pFileRead);
	fclose(pFileRead);
}



float *MultiLinear3DModel::GetCoreCore()
{
	if (m_bUseBlendShape)
	{
		return m_pBlendShape;
	}
	return m_pCoreCore;
}

short *MultiLinear3DModel::GetCore()
{
	return m_pCore;
}

float *MultiLinear3DModel::GetExpress25To47Mat()
{
	return m_pExpress25To47Mat;
}

float *MultiLinear3DModel::GetExpressInitParam()
{
	return m_pExpressInitParam;
}

float *MultiLinear3DModel::GetIdentityInitParam()
{
	return m_pIdentityInitParam;
}

float* MultiLinear3DModel::Get3DFace(float *pIdentityParam, float *pExpressParam)
{
	if (m_bUseBlendShape)
	{
		float *pModel = m_p3DFaceVertex;
		for (int i = 0; i < MLM_FACE_3D_VERTEX; i++)
		{
			float *pExpress = m_pBlendShape + i * 3 * MLM_EXPRESS_DIMS;
			float x = 0.0f, y = 0.0f, z = 0.0f;
			float *p = pExpressParam;
			for (int j = 0; j < MLM_EXPRESS_DIMS; j++)
			{
				x += (*pExpress++)*(*p);
				y += (*pExpress++)*(*p);
				z += (*pExpress++)*(*p++);
			}
			*pModel++ = x * 0.0001f;
			*pModel++ = y * 0.0001f;
			*pModel++ = z * 0.0001f;
		}
	}

	else
	{
		int nFace = MLM_EXPRESS_DIMS * MLM_IDENTITY_DIMS;
		float *pParam = new float[nFace];
		float *p = pParam;
		for (int i = 0; i < MLM_IDENTITY_DIMS; i++)
		{
			float idParam = pIdentityParam[i];
			float *pEp = pExpressParam;
			for (int j = 0; j < MLM_EXPRESS_DIMS; j++)
			{
				*p++ = idParam * (*pEp++);
			}
		}
		short *pCore = m_pCore;
		float *pCoreCore = m_pCoreCore;

		float * p3DFace = m_p3DFaceVertex;
		for (int i = 0; i < MLM_FACE_3D_VERTEX; i++)
		{
			float x = 0.0f, y = 0.0f, z = 0.0f;
			p = pParam;
			{
				x += (*pCoreCore++)*(*p)*MLM_FLOAT_TO_SHORT;
				y += (*pCoreCore++)*(*p)*MLM_FLOAT_TO_SHORT;
				z += (*pCoreCore++)*(*p)*MLM_FLOAT_TO_SHORT;
			}
			for (int j = 0; j < nFace; j++)
			{
				x += (*pCore++)*(*p);
				y += (*pCore++)*(*p);
				z += (*pCore++)*(*p++);
			}
			*p3DFace++ = x*MLM_SHORT_TO_FLOAT;
			*p3DFace++ = y*MLM_SHORT_TO_FLOAT;
			*p3DFace++ = z*MLM_SHORT_TO_FLOAT;
		}
		SAFE_DELETE_ARRAY(pParam);

	}

	for (int i = MLM_FACE_3D_VERTEX; i < MLM_FACE_3D_UV_VERTEX; i++)
	{
		int index3 = m_pUVMap[i - MLM_FACE_3D_VERTEX] * 3;
		int i3 = i * 3;
		m_p3DFaceVertex[i3] = m_p3DFaceVertex[index3];
		m_p3DFaceVertex[i3 + 1] = m_p3DFaceVertex[index3 + 1];
		m_p3DFaceVertex[i3 + 2] = m_p3DFaceVertex[index3 + 2];
	}
	return m_p3DFaceVertex;
}

float *MultiLinear3DModel::GetAndUpdateIdentityDim(short3D *pIndex, float *pExpressParam)
{
	float *pIdentity = m_pIdentityBasis;
	int nFace = MLM_IDENTITY_DIMS*MLM_EXPRESS_DIMS;
	for (int k = 0; k < MLM_LANMARK_INDEX; k++)
	{
		float *pCoreCore = m_pCoreCore + pIndex[k] * 3;
		short *pCore = m_pCore + pIndex[k] * 3 * nFace;
		for (int i = 0; i < MLM_IDENTITY_DIMS; i++)
		{
			float x = 0.0f, y = 0.0f, z = 0.0f;
			float *p = pExpressParam;
			for (int j = 0; j < MLM_EXPRESS_DIMS; j++)
			{
				x += (*pCore++)*(*p);
				y += (*pCore++)*(*p);
				z += (*pCore++)*(*p++);
			}
			*pIdentity++ = x*MLM_SHORT_TO_FLOAT;
			*pIdentity++ = y*MLM_SHORT_TO_FLOAT;
			*pIdentity++ = z*MLM_SHORT_TO_FLOAT;
		}
		{
			m_pIdentityBasis[k * 3 * MLM_IDENTITY_DIMS] += pExpressParam[0] * pCoreCore[0];
			m_pIdentityBasis[k * 3 * MLM_IDENTITY_DIMS + 1] += pExpressParam[0] * pCoreCore[1];
			m_pIdentityBasis[k * 3 * MLM_IDENTITY_DIMS + 2] += pExpressParam[0] * pCoreCore[2];
		}
	}
	return m_pIdentityBasis;
}

float *MultiLinear3DModel::GetAndUpdateExpressDim(short3D *pIndex, float *pIdentityParam)
{
	if (m_bUseBlendShape)
	{
		for (int i = 0; i < MLM_LANMARK_INDEX; i++)
		{
			memcpy(m_pExpressBasis + i*MLM_EXPRESS_DIMS * 3, m_pBlendShape + pIndex[i] * 3 * MLM_EXPRESS_DIMS, sizeof(float)*MLM_EXPRESS_DIMS * 3);
		}
		int nSize = MLM_LANMARK_INDEX*MLM_EXPRESS_DIMS * 3;
		for (int i = 0; i < nSize; i++)
		{
			m_pExpressBasis[i] *= 0.0001f;
		}
		return m_pExpressBasis;
	}
	else
	{
		memset(m_pExpressBasis, 0, sizeof(float)*MLM_LANMARK_INDEX * 3 * MLM_EXPRESS_DIMS);
		int nFace = MLM_EXPRESS_DIMS*MLM_IDENTITY_DIMS;
		for (int k = 0; k < MLM_LANMARK_INDEX; k++)
		{
			float *pExpress = m_pExpressBasis + k * 3 * MLM_EXPRESS_DIMS;
			short *pCore = m_pCore + pIndex[k] * nFace * 3;
			float *pCoreCore = m_pCoreCore + pIndex[k] * 3;
			for (int i = 0; i < MLM_IDENTITY_DIMS; i++)
			{
				float *pExpressi = pExpress;
				float idParam = pIdentityParam[i];
				for (int j = 0; j < MLM_EXPRESS_DIMS; j++)
				{
					(*pExpressi++) += (*pCore++)*idParam*MLM_SHORT_TO_FLOAT;
					(*pExpressi++) += (*pCore++)*idParam*MLM_SHORT_TO_FLOAT;
					(*pExpressi++) += (*pCore++)*idParam*MLM_SHORT_TO_FLOAT;
				}
			}

			{
				m_pExpressBasis[k * 3 * MLM_EXPRESS_DIMS] += pIdentityParam[0] * pCoreCore[0];
				m_pExpressBasis[k * 3 * MLM_EXPRESS_DIMS + 1] += pIdentityParam[0] * pCoreCore[1];
				m_pExpressBasis[k * 3 * MLM_EXPRESS_DIMS + 2] += pIdentityParam[0] * pCoreCore[2];
			}
		}
	return m_pExpressBasis;
	}
	
}

float *MultiLinear3DModel::GetLanmarkVertex(short3D *pIndex, float *pExpressParam, float *pIdentityParam)
{
	if (m_bUseBlendShape)
	{
		float *pLanmark = m_pLanmarkVertex;
		for (int i = 0; i < MLM_LANMARK_INDEX; i++)
		{
			float *pExpress = m_pBlendShape + pIndex[i] * 3 * MLM_EXPRESS_DIMS;
			float x = 0.0f, y = 0.0f, z = 0.0f;
			float *p = pExpressParam;
			for (int j = 0; j < MLM_EXPRESS_DIMS; j++)
			{
				x += (*pExpress++)*(*p);
				y += (*pExpress++)*(*p);
				z += (*pExpress++)*(*p++);
			}
			*pLanmark++ = x * 0.0001f;
			*pLanmark++ = y * 0.0001f;
			*pLanmark++ = z * 0.0001f;
		}
		return m_pLanmarkVertex;
	}


	int nFace = MLM_EXPRESS_DIMS * MLM_IDENTITY_DIMS;
	float *pParam = new float[nFace];
	float *p = pParam;
	for (int i = 0; i < MLM_IDENTITY_DIMS; i++)
	{
		float idParam = pIdentityParam[i];
		float *pEp = pExpressParam;
		for (int j = 0; j < MLM_EXPRESS_DIMS; j++)
		{
			*p++ = idParam * (*pEp++);
		}
	}
	float *pLanmark = m_pLanmarkVertex;
	for (int i = 0; i < MLM_LANMARK_INDEX; i++)
	{
		short *pCore = m_pCore + pIndex[i] * 3 * nFace;
		float *pCoreCore = m_pCoreCore + pIndex[i] * 3;
		float x = 0.0f, y = 0.0f, z = 0.0f;
		p = pParam;
		{
			x += (*pCoreCore++)*(*p)*MLM_FLOAT_TO_SHORT;
			y += (*pCoreCore++)*(*p)*MLM_FLOAT_TO_SHORT;
			z += (*pCoreCore++)*(*p)*MLM_FLOAT_TO_SHORT;
		}
		for (int j = 0; j < nFace; j++)
		{
			x += (*pCore++)*(*p);
			y += (*pCore++)*(*p);
			z += (*pCore++)*(*p++);
		}
		*pLanmark++ = x*MLM_SHORT_TO_FLOAT;
		*pLanmark++ = y*MLM_SHORT_TO_FLOAT;
		*pLanmark++ = z*MLM_SHORT_TO_FLOAT;
	}
	SAFE_DELETE_ARRAY(pParam);
	return m_pLanmarkVertex;
}

float *MultiLinear3DModel::GetAndUpdateContour(float *pIdentityParam, float *pExpressParam)
{
	if (m_bUseBlendShape)
	{
		float *pContour = m_pContourVertex;
		short3D *pIndex = GetContouIndex();
		for (int i = 0; i < MLM_CONTOUR_INDEX; i++)
		{
			float *pExpress = m_pBlendShape + pIndex[i] * 3 * MLM_EXPRESS_DIMS;
			float x = 0.0f, y = 0.0f, z = 0.0f;
			float *p = pExpressParam;
			for (int j = 0; j < MLM_EXPRESS_DIMS; j++)
			{
				x += (*pExpress++)*(*p);
				y += (*pExpress++)*(*p);
				z += (*pExpress++)*(*p++);
			}
			*pContour++ = x * 0.0001f;
			*pContour++ = y * 0.0001f;
			*pContour++ = z * 0.0001f;
		}
		return m_pContourVertex;
	}


	int nFace = MLM_EXPRESS_DIMS * MLM_IDENTITY_DIMS;
	float *pParam = new float[nFace];
	float *p = pParam;
	for (int i = 0; i < MLM_IDENTITY_DIMS; i++)
	{
		float idParam = pIdentityParam[i];
		float *pEp = pExpressParam;
		for (int j = 0; j < MLM_EXPRESS_DIMS; j++)
		{
			*p++ = idParam * (*pEp++);
		}
	}
	float *pContour = m_pContourVertex;
	for (int i = 0; i < MLM_CONTOUR_INDEX; i++)
	{
		short *pCore = m_pCore + m_pContourIndex[i] * 3 * nFace;
		float *pCoreCore = m_pCoreCore + m_pContourIndex[i] * 3;
		float x = 0.0f, y = 0.0f, z = 0.0f;
		p = pParam;
		{
			x += (*pCoreCore++)*(*p)*MLM_FLOAT_TO_SHORT;
			y += (*pCoreCore++)*(*p)*MLM_FLOAT_TO_SHORT;
			z += (*pCoreCore++)*(*p)*MLM_FLOAT_TO_SHORT;
		}
		for (int j = 0; j < nFace; j++)
		{
			x += (*pCore++)*(*p);
			y += (*pCore++)*(*p);
			z += (*pCore++)*(*p++);
		}
		*pContour++ = x*MLM_SHORT_TO_FLOAT;
		*pContour++ = y*MLM_SHORT_TO_FLOAT;
		*pContour++ = z*MLM_SHORT_TO_FLOAT;
	}
	SAFE_DELETE_ARRAY(pParam);
	return m_pContourVertex;

}

short3D *MultiLinear3DModel::GetTriangleIndex()
{
	return m_pTriangleIndex;
}

float *MultiLinear3DModel::GetTextureCoordinates()
{
	return m_pTextureCoordinates;
}
short3D *MultiLinear3DModel::GetFaceTriangleIndex()
{
    return m_pFaceTriangleIndex;
}
    
float *MultiLinear3DModel::GetFaceTextureCoordinates()
{
    return m_pFaceTextureCoordinates;
}

float * MultiLinear3DModel::GetSFMTextureCoordinates()
{
	return m_pSFMTextureCoordinates;
}

float *MultiLinear3DModel::GetFaceNormal()
{
    return m_pFaceNormal;
}

short3D *MultiLinear3DModel::Get3DIndex()
{
	return m_p3Dindex;
}

short3D *MultiLinear3DModel::Get2DIndex()
{
	return m_p2Dindex;
}

short3D *MultiLinear3DModel::GetContouIndex()
{
	return m_pContourIndex;
}

short3D *MultiLinear3DModel::GetContourLine()
{
	return m_pContourLine;
}

short3D*MultiLinear3DModel::GetUVMap()
{
	return m_pUVMap;
}



void MultiLinear3DModel::ObjOut(float *pModel3D, float *pTextPos, int nTextWidth, int nTextHeight, char *pImageName, bool bNormal /* = false */)
{
	freopen("MT3DMaterial/out.obj", "w", stdout);

	//输出顶点、纹理坐标、三角形顶点索引
	printf("mtllib out.mtl\n");
	for (int i = 0; i < MLM_FACE_3D_UV_VERTEX; i++)
	{
		printf("v %f %f %f\n", pModel3D[i * 3], pModel3D[i * 3 + 1], pModel3D[i * 3 + 2]);
	}
	if (bNormal)
	{
		for (int i = 0; i < MLM_FACE_3D_UV_VERTEX; i++)
		{
			printf("vt %f %f\n", pTextPos[2 * i], pTextPos[i * 2 + 1]);
		}
	}
	else
	{
		for (int i = 0; i < MLM_FACE_3D_UV_VERTEX; i++)
		{
			printf("vt %f %f\n", pTextPos[2 * i] / nTextWidth, 1.0f - pTextPos[i * 2 + 1] / nTextHeight);
		}
	}

	printf("usemtl FaceTexture\n");
	short3D *pT = GetTriangleIndex();
	int nTriangleNum = MLM_TRIANGLE_NUM;
	for (int i = 0; i < nTriangleNum; i++)
	{
		int i3 = i * 3;
		printf("f %d/%d %d/%d %d/%d\n", pT[i3] + 1, pT[i3] + 1, pT[i3 + 1] + 1, pT[i3 + 1] + 1, pT[i3 + 2] + 1, pT[i3 + 2] + 1);
	}
	fclose(stdout);
	freopen("MT3DMaterial/out.mtl", "w", stdout);
	printf("newmtl FaceTexture\n");
	printf("map_Kd %s\n", pImageName);

	fclose(stdout);
	return;
}
} // namespace mt3dface
