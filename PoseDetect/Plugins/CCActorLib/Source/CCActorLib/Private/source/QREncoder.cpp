#include "common/QREncoder.h"
#include "common/QR_Encode.h"
//#include "common/Bitmap.h"
#include "Engine/Texture2D.h"
#include "win/win32.h"
#include "Engine/Classes/Engine/Texture2D.h"



template <typename T>
inline const T qBound(const T &min, const T &val, const T &max)
{
	return FMath::Max(min, FMath::Min(max, val));
}

UTexture2D* QREncoder::makeImage(const FString& encodeString, const FVector2D size,int imageMargin /*= 2*/, int level /*= 1*/, int version /*= 0*/, bool autoExtent /*= true*/, int maskingNo /*= -1*/)
{

	//level = qBound(0, level, 3);
	//version = qBound(0, version, 40);
	//maskingNo = qBound(-1, maskingNo, 7);
	//CQR_Encode* encode = new CQR_Encode;	//尽量不在栈中生成
	//if (!encode->EncodeData(level, version, autoExtent, maskingNo, *encodeString,0))
	//{
	//	delete encode;
	//	return nullptr;
	//}
	//Bitmap img = Bitmap(encode->m_nSymbleSize + imageMargin * 2, encode->m_nSymbleSize + imageMargin * 2, Bitmap::Format_RGBA);
	//for (size_t row = 0; row < img.height(); ++row)
	//{
	//	for (size_t col = 0; col < img.width(); ++col )
	//	{
	//		unsigned char buf[] = { 255,255,255,255};
	//		img.setPixel(col, row, &buf[0]);
	//	}
	//}

	////QImage img(encode->m_nSymbleSize + imageMargin * 2, encode->m_nSymbleSize + imageMargin * 2, QImage::Format_Mono);
	////img.fill(1);
	//for (int row = 0; row < encode->m_nSymbleSize; row++)
	//{
	//	for (int col = 0; col < encode->m_nSymbleSize; col++)
	//	{
	//		if (encode->m_byModuleData[col][row])
	//		{
	//			unsigned char buf[] = { 0.0f,0.0f,0.0f,255 };
	//			img.setPixel(col + imageMargin, row + imageMargin, &buf[0]);
	//		}
	//	}
	//}

	////saveAsBMP((char*)img.pixelBuffer(), img.width()*img.height() * 4, img.width(), img.height(), 32, L"d://qrcode.bmp");

	//delete encode;

	//Bitmap* targetImg = new Bitmap(size.X, size.Y, Bitmap::Format_RGBA);
	//*targetImg = img.scale(size.X, size.Y);

	UTexture2D *QRTex = UTexture2D::CreateTransient(1920, 1080);
	//QRTex->UpdateResource();

	//struct FUpdateTextureRegionsData
	//{
	//	FTexture2DResource* Texture2DResource;
	//	int32 MipIndex;
	//	FUpdateTextureRegion2D Region;
	//	uint32 SrcPitch;
	//	uint32 SrcBpp;
	//	uint8* SrcData;
	//};

	//FUpdateTextureRegionsData* RegionData = new FUpdateTextureRegionsData;

	//RegionData->Texture2DResource = QRTex->Resource->GetTexture2DResource();
	//RegionData->MipIndex = 0;
	//RegionData->Region = FUpdateTextureRegion2D(0, 0, 0, 0, targetImg->width(), targetImg->height());
	//RegionData->SrcPitch = targetImg->width() * 4;
	//RegionData->SrcBpp = (uint32)4;
	//RegionData->SrcData = (uint8*)targetImg->pixelBuffer();

	//ENQUEUE_RENDER_COMMAND(UpdateTextureRegionsData)(
	//	[RegionData, targetImg](FRHICommandListImmediate& RHICmdList)
	//{
	//	int32 CurrentFirstMip = RegionData->Texture2DResource->GetCurrentFirstMip();
	//	if (RegionData->MipIndex >= CurrentFirstMip)
	//	{
	//		RHIUpdateTexture2D(RegionData->Texture2DResource->GetTexture2DRHI(),
	//			RegionData->MipIndex - CurrentFirstMip,
	//			RegionData->Region,
	//			RegionData->SrcPitch,
	//			RegionData->SrcData + RegionData->Region.SrcY * RegionData->SrcPitch + RegionData->Region.SrcX * RegionData->SrcBpp);
	//	}

	//	delete RegionData;
	//	delete targetImg;
	//});//End Enqueue

	return QRTex;
}

#ifdef WINDOWS_PLATFORM_TYPES_GUARD
#include "Windows/HideWindowsPlatformTypes.h"
#endif