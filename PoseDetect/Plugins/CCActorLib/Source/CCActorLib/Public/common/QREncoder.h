#pragma once
#include <Runtime/Core/Public/CoreMinimal.h>

class UTexture2D;
class CCACTORLIB_API QREncoder
{
public:
	//************************************
	// Method:    makeImage
	// FullName:  QREncoder::makeImage
	// Access:    public static 
	// Returns:   QT_NAMESPACE::QImage
	// Qualifier:
	// Parameter: const QString & encodeString
	// Parameter: int imageMargin		图像周围的边框;
	// Parameter: int level				纠错等级.	0 - 3: 7% 15% 25% 30%;
	// Parameter: int version			版本.		0 - 40: 0 = auto
	// Parameter: bool autoExtent		自动匹配.
	// Parameter: int maskingNo			掩模图型.	-1 - 7: -1 = auto
	//************************************
	static UTexture2D* makeImage(const FString& encodeString, const FVector2D size, int imageMargin = 2, int level = 1, int version = 0, bool autoExtent = true, int maskingNo = -1);

protected:
	QREncoder(void) {}
	~QREncoder(void) {}
};
