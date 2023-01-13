#pragma once
#include "inc.h"

enum class EventType :int8_t
{
	ChangeVideo = 0,
	ChangeImage,
	ChangeCamera,
	OpenGLBTask,
	ChangeAREffect,
	ChangeXML,
	ReSize,
	MouseEvent,

	OpenAnimateJson,
	OpenFBX,
	AnimateSLerp,
	RotateHDR,
	LightInfo,
	ChangeFurData,
	DynamicBone,
	ChangeTonemapping,
	Bloom,
	ChangePreviewVideo,
	ChangeHDRFile,
	GetCurrentEffectAttr,
	SetEffectAttr,
	OpenXMLFile,
	ExportXMLConfig,
	EventTypeNum,
};

// EventTypeMax == EventType::EventTypeNum-1
#define EventTypeMax 22
#define AddEventInfo(ClassType) \
static_assert(EventTypeMax ==(int(EventType::EventTypeNum)-1),"EventTypeMax must  equal EventTypeNum-1");\
AddEventFunction<ClassType, EventTypeMax>();  \

struct TaskEvent
{
	TaskEvent(EventType _type):type(_type) {};
	virtual ~TaskEvent() {}

	EventType type;
};



/// Help
template<class T,int n>
inline void AddEventFunction()
{
	if (n>=0)
	{
		T::EventMap[n] = &T::OnEvent<n>;
		AddEventFunction<T, n - 1>();
	}
}

#define DeclarationOnEvent(ClassType) \
virtual void Event(std::shared_ptr<TaskEvent> E)\
{\
	EventDispatch;\
};\
template<int Type> \
void OnEvent(std::shared_ptr<TaskEvent> E) {};\
typedef void(ClassType::* EventFunc)(std::shared_ptr<TaskEvent> E); \
static std::map<int, EventFunc> EventMap;\


#define EventDispatch \
if (EventMap.find((int)E->type)!= EventMap.cend())\
{\
	(this->*EventMap[(int)E->type])(E);\
}\

#define ImplementEvent(ClassType) \
std::map<int, ClassType::EventFunc> ClassType::EventMap= std::map<int, ClassType::EventFunc>();\
template<>\
inline void AddEventFunction<ClassType, -1>()\
{\
}\
class ClassType##_HelpEvent\
{\
public:\
	ClassType##_HelpEvent()\
	{\
		AddEventInfo(ClassType);\
	}\
	static int Init()\
	{\
		static ClassType##_HelpEvent Obj;\
		return 0;\
	}\
};\
int ClassType##_HelpEventInitNum = ClassType##_HelpEvent::Init();\
\
