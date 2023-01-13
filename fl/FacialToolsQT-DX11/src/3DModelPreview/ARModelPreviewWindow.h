#pragma once

#include <QtWidgets/qmainwindow.h>
#include <QtCore>
#include <QEvent>
#include "ui_ARModelPreviewWindow.h"

class ModelRenderLogic;
class BasicPreivew;
struct TaskEvent;
class BaseToolTab;
class MaterialToolTab;
class ProcesProcessToolTab;

class ARModelPreviewWindow : public QMainWindow
{
	Q_OBJECT

public:
	ARModelPreviewWindow(QWidget* parent);
	~ARModelPreviewWindow();

	void StartPreview();
	void PostEvent(std::shared_ptr<TaskEvent> event);
	void ResetModelPostion();
	std::shared_ptr< BasicPreivew> GetPreview() const { return m_RenderPreview; }

private:
	virtual void mousePressEvent(QMouseEvent* event);
	virtual void mouseMoveEvent(QMouseEvent* event);
	virtual void wheelEvent(QWheelEvent* event);
	virtual void resizeEvent(QResizeEvent* event) override;

	// ！！ events ！！！！！！！！！！！！！！！！！！！！！！！！！！！
  /*
   * this event is called when the mouse enters the widgets area during a drag/drop operation
   */
	void dragEnterEvent(QDragEnterEvent* event);

	/**
	 * this event is called when the mouse moves inside the widgets area during a drag/drop operation
	 */
	void dragMoveEvent(QDragMoveEvent* event);

	/**
	 * this event is called when the mouse leaves the widgets area during a drag/drop operation
	 */
	void dragLeaveEvent(QDragLeaveEvent* event);

	/**
	 * this event is called when the drop operation is initiated at the widget
	 */
	void dropEvent(QDropEvent* event);

	virtual bool event(QEvent* e);

public:
	Ui::ARModelPreviewWindow ModelUI;
private:
	
	QPointer<QFrame> m_Frame;
	std::shared_ptr< ModelRenderLogic> m_RenderLogic;
	std::shared_ptr< BasicPreivew> m_RenderPreview;
	Qt::MouseButton m_MouseButton;
	QPointer< BaseToolTab> m_BaseToolTab;
	QPointer< MaterialToolTab> m_MaterialToolTab;
	QPointer< ProcesProcessToolTab> m_PostProcessToolTab;
	float VideoScale = 1.0f;
};