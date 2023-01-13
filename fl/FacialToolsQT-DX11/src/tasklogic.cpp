#include "tasklogic.h"
#include "d3d11renderwidget.h"
#include "ARZipPreviewWindow.h"
#include <QtCore/QCoreApplication>

ImplementEvent(TaskLogic);

TaskLogic::TaskLogic(D3d11RenderWidget* RenderWidget)
	:mRenderWidget(RenderWidget)
{

}

TaskLogic::~TaskLogic()
{

}

template<> void TaskLogic::OnEvent<EventType::ChangeVideo>(std::shared_ptr<TaskEvent> E)
{
	std::shared_ptr< ChangeVideo> ChangeVideoTask = std::static_pointer_cast<ChangeVideo>(E);
	if (mRenderWidget)
	{
		mRenderWidget->changeVideoFileType(ChangeVideoTask->fileName);
	}
}

template<> void TaskLogic::OnEvent<EventType::OpenGLBTask>(std::shared_ptr<TaskEvent> E)
{
	std::shared_ptr< OpenGLBTask> Change3DEffectTask = std::static_pointer_cast<OpenGLBTask>(E);
	if (mRenderWidget)
	{
		mRenderWidget->changeGLBResource(Change3DEffectTask->fileName);

		SwitchLayoutEvent* e = new SwitchLayoutEvent();
		qApp->postEvent(mRenderWidget->parent(), e);
	}
}

template<> void TaskLogic::OnEvent<EventType::ChangeImage>(std::shared_ptr<TaskEvent> E)
{
	std::shared_ptr< ChangeImage> ChangeTask = std::static_pointer_cast<ChangeImage>(E);
	if (mRenderWidget)
	{
		mRenderWidget->changeImageFileType(ChangeTask->fileName);
	}
}

template<> void TaskLogic::OnEvent<EventType::ChangeCamera>(std::shared_ptr<TaskEvent> E)
{
	std::shared_ptr< ChangeCamera> ChangeTask = std::static_pointer_cast<ChangeCamera>(E);
	if (mRenderWidget)
	{
		mRenderWidget->changeCameraFileType();
	}
}

template<> void TaskLogic::OnEvent<EventType::ChangeAREffect>(std::shared_ptr<TaskEvent> E)
{
	std::shared_ptr< ChangeAREffect> ChangeAREffectTask = std::static_pointer_cast<ChangeAREffect>(E);
	if (mRenderWidget)
	{
		mRenderWidget->changeEffectResource(ChangeAREffectTask->fileName);

		SwitchLayoutEvent* e = new SwitchLayoutEvent();
		qApp->postEvent(mRenderWidget->parent(), e);
	}
}

template<> void TaskLogic::OnEvent<EventType::ChangeXML>(std::shared_ptr<TaskEvent> E)
{
	std::shared_ptr< ChangeXMLEffect> ChangeXMLEffectTask = std::static_pointer_cast<ChangeXMLEffect>(E);
	if (mRenderWidget)
	{
		mRenderWidget->changeEffectResourceXML(ChangeXMLEffectTask->fileName);

		SwitchLayoutEvent* e = new SwitchLayoutEvent();
		qApp->postEvent(mRenderWidget->parent(), e);
	}
}

template<>
void TaskLogic::OnEvent<EventType::OpenAnimateJson>(std::shared_ptr<TaskEvent> E)
{
	std::shared_ptr< OpenAnimateJsonEvent> openJsonEvent = std::static_pointer_cast<OpenAnimateJsonEvent>(E);
	if (mRenderWidget)
	{
		mRenderWidget->LoadAnimateJson(openJsonEvent->fileName.toUtf8().data());
	}
	
}

template<> void TaskLogic::OnEvent<EventType::ReSize>(std::shared_ptr<TaskEvent> E)
{
	std::shared_ptr< ReSizeEvent> ReSizeTask = std::static_pointer_cast<ReSizeEvent>(E);
	if (mRenderWidget)
	{
		mRenderWidget->ReSize(ReSizeTask->width, ReSizeTask->height);
	}
}

template<> void TaskLogic::OnEvent<EventType::MouseEvent>(std::shared_ptr<TaskEvent> E)
{
	std::shared_ptr< QTMouseEvent> Event = std::static_pointer_cast<QTMouseEvent>(E);
	if (mRenderWidget)
	{
		mRenderWidget->execMouseEvent(Event->Scale,Event->Rotate,Event->Translate);
	}
}
