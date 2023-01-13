#include "BasicPreview.h"
#include <QFrame>
#include "ModelRenderLogic.h"
#include "Task/basetask.h"
#include "Toolbox/Render/DynamicRHI.h"

BasicPreivew::BasicPreivew(QObject* parent /*= nullptr*/)
	:QObject(parent)
{

}

BasicPreivew::~BasicPreivew()
{

}

void BasicPreivew::StartPreview(QWidget* frame, std::shared_ptr<ModelRenderLogic> Logic)
{
	m_ParentFrame = frame;
	m_RenderLogic = Logic;
	m_WndId = (void*)frame->winId();
	m_FrameRect = frame->rect();
	if (!m_RenderThread.joinable())
	{
		
		m_RenderThread = std::thread(std::bind(&BasicPreivew::RenderThread, this));
	}
}

void BasicPreivew::StopPreview()
{
	m_Exit = true;
	if (m_RenderThread.joinable())
	{
		m_RenderThread.join();
	}
}

void BasicPreivew::MousePress(Qt::MouseButton button, QPoint pos)
{
	if (button == Qt::MouseButton::LeftButton)
	{
		m_LBDownPoint = pos;
		m_LBtnDownTranslate = m_Rotate;
	}
	else if (button == Qt::MouseButton::RightButton)
	{
		m_RBDownPoint = pos;
		m_RBtnDownTranslate = m_Translate;
	}
}

void BasicPreivew::MouseMove(Qt::MouseButton button, QPoint pos)
{
	if (button == Qt::MouseButton::LeftButton)
	{
		m_Rotate = m_LBtnDownTranslate + core::vec2f(pos.x() - m_LBDownPoint.x(), (pos.y() - m_LBDownPoint.y()));
	}
	if (button == Qt::MouseButton::RightButton)
	{
		m_Translate = m_RBtnDownTranslate + core::vec2f(pos.x() - m_RBDownPoint.x(), (pos.y() - m_RBDownPoint.y()));
	}

	MakeMouseEvent(0.0f, core::vec2f(m_Rotate.x, m_Rotate.y), core::vec2f(m_Translate.x, m_Translate.y));
}

void BasicPreivew::Wheel(int Delta)
{
	float TempScale = 0.0;
	if (Delta < 0)
	{
		TempScale = -0.2;
	}
	else
	{
		TempScale = 0.2;
	}
	MakeMouseEvent(TempScale, m_Rotate, m_Translate);
}

void BasicPreivew::PostEvent(std::shared_ptr<TaskEvent> event)
{
	std::lock_guard<std::recursive_mutex> Lock(m_TaskLock);
	m_EventList.append(event);
}

void BasicPreivew::ResetModelPostion()
{
	m_Rotate = {};
	m_Translate = {};
	m_LBtnDownTranslate = {};
	m_RBtnDownTranslate = {};
	m_LBDownPoint = {};
	m_RBDownPoint = {};
	MakeMouseEvent(0, m_Rotate, m_Translate);
}

void BasicPreivew::ReSizeRenderFrame(QSize size)
{
	std::lock_guard<std::mutex> lock(m_SizeLock);
	m_FrameRect.setSize(size);
	m_Resize = true;

}

void BasicPreivew::ResetMousePos()
{
	m_LBDownPoint = {};
	m_RBDownPoint = {};
	m_Rotate = core::vec2f();
	m_LBtnDownTranslate = core::vec2f();
	m_RBtnDownTranslate = core::vec2f();
	m_Translate = core::vec2f();
}

void BasicPreivew::RenderThread()
{
	if (!Init())
	{
		return;
	}

	DoPrepare();

	std::chrono::high_resolution_clock::time_point tStart, tEnd;
	tStart = std::chrono::high_resolution_clock::now();
	while (!m_Exit)
	{
		RunTask();

		tEnd = std::chrono::high_resolution_clock::now();
		float Delta = std::chrono::duration<float, std::milli>(tEnd - tStart).count();
		if (Delta < (1000.0f / 90.f))
		{
			std::this_thread::sleep_for(1ms);
			continue;
		}

		DoRender();
		tStart = std::chrono::high_resolution_clock::now();
	}

	UnInit();
}

bool BasicPreivew::Init()
{
	return false;
}

void BasicPreivew::UnInit()
{
	if (m_RenderLogic)
	{
		m_RenderLogic->ShutDown();
	}
	
}

void BasicPreivew::DoRender()
{

}

void BasicPreivew::DoPrepare()
{
	if (m_RenderLogic)
	{
		m_RenderLogic->Prepare();
	}
	
}

void BasicPreivew::RunTask()
{
	if (!m_RenderLogic)
	{
		return;
	}

	QList<std::shared_ptr<TaskEvent>> EventList;
	{
		std::lock_guard<std::recursive_mutex> Lock(m_TaskLock);
		if (m_EventList.isEmpty())
		{
			return;
		}
		EventList.swap(m_EventList);
	}

	if (EventList.isEmpty())
	{
		return;
	}

	do
	{
		std::shared_ptr<TaskEvent> Event = EventList.front();
		EventList.pop_front();

		m_RenderLogic->Event(Event);

	} while (!EventList.isEmpty());
}

void BasicPreivew::MakeMouseEvent(float Scale, const core::vec2f& Rotate, const core::vec2f& Translate)
{
	std::shared_ptr<QTMouseEvent> MouseEvent = std::make_shared<QTMouseEvent>();
	MouseEvent->Scale = Scale;
	MouseEvent->Rotate = Rotate;
	MouseEvent->Translate = Translate;

	PostEvent(std::static_pointer_cast<TaskEvent>(MouseEvent));
}
