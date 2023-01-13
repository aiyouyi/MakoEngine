#pragma once
#include "inc.h"
#include "Toolbox/vec2.h"
#include <QtCore>

class QWidget;
struct TaskEvent;
class ModelRenderLogic;

class BasicPreivew : public QObject
{
	Q_OBJECT
public:
	BasicPreivew(QObject* parent = nullptr);
	~BasicPreivew();

	void StartPreview(QWidget* frame, std::shared_ptr<ModelRenderLogic> Logic);
	void StopPreview();

	void MousePress(Qt::MouseButton button, QPoint pos);
	void MouseMove(Qt::MouseButton button, QPoint pos);
	void Wheel(int Delta);

	void PostEvent(std::shared_ptr<TaskEvent> event);
	void ResetModelPostion();
	core::vec2f GetTranslate() { return m_Translate; }
	void ReSizeRenderFrame(QSize size);
	void ResetMousePos();

protected:
	void  RenderThread();
	virtual bool Init();
	virtual void UnInit();
	virtual void DoRender();
	virtual void DoPrepare();
	void RunTask();
	void MakeMouseEvent(float Scale, const core::vec2f& Rotate, const core::vec2f& Translate);

protected:
	bool m_Exit = false;
	QPointer<QWidget> m_ParentFrame;
	std::shared_ptr<ModelRenderLogic> m_RenderLogic;
	void* m_WndId = nullptr;
	QRect m_FrameRect;
	std::thread m_RenderThread;
	std::recursive_mutex m_TaskLock;
	QPoint m_LBDownPoint;
	QPoint m_RBDownPoint;
	core::vec2f m_Rotate;
	core::vec2f m_LBtnDownTranslate;
	core::vec2f m_RBtnDownTranslate;
	core::vec2f m_Translate;
	QList<std::shared_ptr<TaskEvent>> m_EventList;
	std::atomic_bool m_Resize;
	std::mutex m_SizeLock;
};