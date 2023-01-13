#pragma once
#include "inc.h"
#include "CC3DEngine/RenderCommon/PBRRenderDef.h"
#include "CC3DEngine/CC3DConfigStream.h"
#include "CC3DEngine/Material/FurConfig.h"
#include "CC3DEngine/Material/CC3DPbrMaterial.h"
#include "CC3DEngine/Skeleton/DynamicBoneInfo.h"

class DataCenter
{
public:
	DataCenter();
	~DataCenter();
public:
	static DataCenter& getInstance();

	CC3DImageFilter::ModelConfig RecordInfo;
	CC3DImageFilter::FurConfigData FurData;
	CC3DImageFilter::CC3DConfigStream Config_stream;
};