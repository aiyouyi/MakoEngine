#pragma once
#include "taskevent.h"
#include "Toolbox/vec2.h"
#include "BaseDefine/Vectors.h"
#include <QString>
#include<functional>

struct ChangeVideo : public TaskEvent
{
	ChangeVideo() : TaskEvent(EventType::ChangeVideo) {}
	virtual ~ChangeVideo() {}
	QString fileName;
};

struct OpenGLBTask : public TaskEvent
{
	OpenGLBTask() : TaskEvent(EventType::OpenGLBTask) {}
	virtual ~OpenGLBTask() {}
	QString fileName;
};

struct ChangeImage : public TaskEvent
{
	ChangeImage() : TaskEvent(EventType::ChangeImage){}
	virtual ~ChangeImage(){}
	QString fileName;
};

struct OpenAnimateJsonEvent : public TaskEvent
{
	OpenAnimateJsonEvent() : TaskEvent(EventType::OpenAnimateJson) {}
	virtual ~OpenAnimateJsonEvent() {}
	QString fileName;
};

struct ChangeCamera : public TaskEvent
{
	ChangeCamera() : TaskEvent(EventType::ChangeCamera) {}
	virtual ~ChangeCamera() {}
};

struct ChangeAREffect : public TaskEvent
{
	ChangeAREffect() : TaskEvent(EventType::ChangeAREffect) {}
	virtual ~ChangeAREffect() {}
	QString fileName;
};

struct ChangeXMLEffect : public TaskEvent
{
	ChangeXMLEffect() : TaskEvent(EventType::ChangeXML) {}
	virtual ~ChangeXMLEffect() {}
	QString fileName;
};

struct ReSizeEvent : public TaskEvent
{
	ReSizeEvent() : TaskEvent(EventType::ReSize) {}
	virtual ~ReSizeEvent() {}
	uint32_t width = 0;
	uint32_t height = 0;
};

struct QTMouseEvent : public TaskEvent
{
	QTMouseEvent() :TaskEvent(EventType::MouseEvent) {}
	virtual ~QTMouseEvent() {}
	float Scale = 0.;
	core::vec2f Rotate;
	core::vec2f Translate;
};

struct RotateHDREvent : public TaskEvent
{
	RotateHDREvent() : TaskEvent(EventType::RotateHDR) {}
	virtual ~RotateHDREvent() {}

	float hdrRotateX = 0.0;
	float hdrRotateY = 0.0;
	float hdrScale = 1.0f;
	float hdrContrast = 1.0f;
};

struct LightInfoEvent : public TaskEvent
{
	LightInfoEvent() : TaskEvent(EventType::LightInfo) {}
	virtual ~LightInfoEvent() {}

};

struct ChangeToneMappingEvent : public TaskEvent
{
	ChangeToneMappingEvent() : TaskEvent(EventType::ChangeTonemapping) {}
	virtual ~ChangeToneMappingEvent() {}

};

struct FurDataEvent : public TaskEvent
{
	FurDataEvent() : TaskEvent(EventType::ChangeFurData) {}
	virtual ~FurDataEvent() {}
};

enum class DynamicBoneOperateType
{
	ADD_DB,
	DELETE_DB,
	CLEAR_DB,
	Update_DB,
};

struct DynamicBoneEvent : public TaskEvent
{
	DynamicBoneEvent() : TaskEvent(EventType::DynamicBone) {}
	virtual ~DynamicBoneEvent() {}
	DynamicBoneOperateType dbType;
	QString dbName;
};

struct BloomEvent : public TaskEvent
{
	BloomEvent() : TaskEvent(EventType::Bloom) {}
	virtual ~BloomEvent() {}
};

struct VideoPreviewEvent : public TaskEvent
{
	VideoPreviewEvent() : TaskEvent(EventType::ChangePreviewVideo) {}
	virtual ~VideoPreviewEvent() {}
	QString FileName;
	bool UseVideo = false;
};

struct ChangeHDRFileEvent : public TaskEvent
{
	ChangeHDRFileEvent() : TaskEvent(EventType::ChangeHDRFile) {}
	virtual ~ChangeHDRFileEvent() {}
	QString FileName;
};

struct EffectAttrData
{
	QString ShowName;
	float Alpha;
	void* pEffect;
};

struct GetCurrentEffectAttrEvent : public TaskEvent
{
	GetCurrentEffectAttrEvent() : TaskEvent(EventType::GetCurrentEffectAttr) {}
	virtual ~GetCurrentEffectAttrEvent() {}
	std::vector<EffectAttrData> Effects;
	std::function<void()> FinishCallBack;
};

struct SetEffectAttrEvent : public TaskEvent
{
	SetEffectAttrEvent() : TaskEvent(EventType::SetEffectAttr) {}
	virtual ~SetEffectAttrEvent() {}
	EffectAttrData AttrData;
};

struct OepnXMLFileEvent : public TaskEvent
{
	OepnXMLFileEvent() : TaskEvent(EventType::OpenXMLFile) {}
	virtual ~OepnXMLFileEvent() {}
	QString FileName;
};

struct ExportXMLConfigEvent : public TaskEvent
{
	ExportXMLConfigEvent() : TaskEvent(EventType::ExportXMLConfig) {}
	virtual ~ExportXMLConfigEvent() {}
	QString FileName;
};