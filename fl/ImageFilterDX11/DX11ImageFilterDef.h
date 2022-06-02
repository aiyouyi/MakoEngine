#ifndef DX11ImageFilterDef_h
#define DX11ImageFilterDef_h

#ifdef _MSC_VER  
#ifdef  DX11IMAGEFILTER_EXPORTS
#define DX11IMAGEFILTER_EXPORTS_API extern "C" __declspec(dllexport)
#define DX11IMAGEFILTER_EXPORTS_CLASS __declspec(dllexport)
#else
#define DX11IMAGEFILTER_EXPORTS_API extern "C" __declspec(dllimport)
#define DX11IMAGEFILTER_EXPORTS_CLASS __declspec(dllimport)
#endif

#ifdef  MAGICSTUDIO_EXPORTS
#define MAGICSTUDIO_EXPORTS_CLASS __declspec(dllexport)
#else
#define MAGICSTUDIO_EXPORTS_CLASS 
#endif

#else

#ifdef __cplusplus
#define DX11IMAGEFILTER_EXPORTS_API extern "C"
#define DX11IMAGEFILTER_EXPORTS_CLASS
#else
#define DX11IMAGEFILTER_EXPORTS_API
#define DX11IMAGEFILTER_EXPORTS_CLASS
#endif

#endif

#endif
