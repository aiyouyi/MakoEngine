#pragma once
#include "Task/basetask.h"
#include <QPointer>

class D3d11RenderWidget;
class TaskLogic
{
public:
	TaskLogic(D3d11RenderWidget *RenderWidget);
	~TaskLogic();

	DeclarationOnEvent(TaskLogic);
	template<> void OnEvent<EventType::ChangeVideo>(std::shared_ptr<TaskEvent> E);
	template<> void OnEvent<EventType::OpenGLBTask>(std::shared_ptr<TaskEvent> E);
	template<> void OnEvent<EventType::ChangeImage>(std::shared_ptr<TaskEvent> E);
	template<> void OnEvent<EventType::ChangeCamera>(std::shared_ptr<TaskEvent> E);
	template<> void OnEvent<EventType::ChangeAREffect>(std::shared_ptr<TaskEvent> E);
	template<> void OnEvent<EventType::ChangeXML>(std::shared_ptr<TaskEvent> E);
	template<> void OnEvent<EventType::ReSize>(std::shared_ptr<TaskEvent> E);
	template<> void OnEvent<EventType::MouseEvent>(std::shared_ptr<TaskEvent> E);
	template<> void OnEvent<EventType::OpenAnimateJson>(std::shared_ptr<TaskEvent> E);
private:
	QPointer< D3d11RenderWidget> mRenderWidget;
};