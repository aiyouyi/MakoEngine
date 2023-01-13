#include "ARModelPreviewWindow.h"
#include "ModelRenderLogic.h"
#include "D3D11Preview.h"
#include "Toolbox/Render/DynamicRHI.h"
#include "BasicToolTab.h"
#include "MaterialToolTab.h"
#include "PostProcessToolTab.h"
#include "QUIEvent.h"
#include <QtGui>
#include <QSizePolicy>

ARModelPreviewWindow::ARModelPreviewWindow(QWidget* parent)
	:QMainWindow(parent)
{
	ModelUI.setupUi(this);
	m_Frame = ModelUI.frame;
	m_Frame->setAttribute(Qt::WA_NoSystemBackground);
	m_Frame->setAttribute(Qt::WA_UpdatesDisabled);

	QSize s = size();
	setMinimumSize(s.width(), s.height());

	m_RenderLogic = std::make_shared<ModelRenderLogic>(this);

	if (GetDynamicRHI()->API == CC3DDynamicRHI::DX11)
	{
		m_RenderPreview = std::make_shared<D3D11Preview>(this);
	}

	m_BaseToolTab = new BaseToolTab(ModelUI.BaseTool, this);
	m_MaterialToolTab = new MaterialToolTab(ModelUI.MaterialTool, this);
	m_PostProcessToolTab = new ProcesProcessToolTab(ModelUI.PostProcessTool, this);
	ModelUI.tabWidget->setCurrentIndex(0);

	setAcceptDrops(true);
}

ARModelPreviewWindow::~ARModelPreviewWindow()
{
	if (m_RenderPreview)
	{
		m_RenderPreview->StopPreview();
	}
	m_RenderPreview = {};
}

void ARModelPreviewWindow::StartPreview()
{
	if (m_RenderPreview)
	{
		m_RenderPreview->StartPreview(m_Frame, m_RenderLogic);
	}
}

void ARModelPreviewWindow::PostEvent(std::shared_ptr<TaskEvent> event)
{
	if (m_RenderPreview)
	{
		m_RenderPreview->PostEvent(event);
	}
}

void ARModelPreviewWindow::ResetModelPostion()
{
	if (m_RenderPreview)
	{
		m_RenderPreview->ResetModelPostion();
	}
}

void ARModelPreviewWindow::mousePressEvent(QMouseEvent* event)
{
	if (m_Frame->frameGeometry().contains(event->pos()))
	{
		m_RenderPreview->MousePress(event->button(), m_Frame->mapFromParent(event->pos()));
	}
	m_MouseButton = event->button();
	QWidget::mousePressEvent(event);
}

void ARModelPreviewWindow::mouseMoveEvent(QMouseEvent* event)
{
	if (m_Frame->frameGeometry().contains(event->pos()))
	{
		m_RenderPreview->MouseMove(m_MouseButton, m_Frame->mapFromParent(event->pos()));
		m_RenderLogic->SetVideoTranslate(m_RenderPreview->GetTranslate());
	}
	QWidget::mouseMoveEvent(event);
}

void ARModelPreviewWindow::wheelEvent(QWheelEvent* event)
{
	if (event->delta() < 0)
	{
		VideoScale -= 0.1f;
		if (VideoScale < 0.1)
		{
			VideoScale = 0.1;
		}
	}
	else
	{
		VideoScale += 0.1f;
	}
	

	if (m_Frame->frameGeometry().contains(event->pos()))
	{
		m_RenderLogic->SetVideoScale(VideoScale);
		m_RenderPreview->Wheel(event->delta());
	}
	QWidget::wheelEvent(event);
}

void ARModelPreviewWindow::resizeEvent(QResizeEvent* event)
{
	int TabWidgetWidth = ModelUI.tabWidget->width();
	ModelUI.tabWidget->setFixedHeight(event->size().height());
	ModelUI.frame->setFixedSize(event->size().width() - TabWidgetWidth, event->size().height());
	m_RenderPreview->ReSizeRenderFrame(ModelUI.frame->size());
	QWidget::resizeEvent(event);
}

void ARModelPreviewWindow::dragEnterEvent(QDragEnterEvent* event)
{
	const QMimeData* mimeData = event->mimeData();

	if (mimeData->hasUrls())
	{
		QStringList pathList;
		QList<QUrl> urlList = mimeData->urls();

		for (int i = 0; i < urlList.size(); ++i)
		{
			pathList.append(urlList.at(i).toLocalFile());
		}

		bool HasValidFile = false;
		for (size_t index = 0; index < pathList.size(); ++index)
		{
			QString FileName = pathList[index];

			if (FileName.contains(".mp4", Qt::CaseInsensitive) || FileName.contains(".mkv", Qt::CaseInsensitive) 
				|| FileName.contains(".mov", Qt::CaseInsensitive) || FileName.contains(".flv", Qt::CaseInsensitive) ||
				FileName.contains(".avi", Qt::CaseInsensitive))
			{
				HasValidFile = true;
			}
			else if (FileName.contains(".zip", Qt::CaseInsensitive))
			{
				HasValidFile = true;
			}
			else if (FileName.contains(".hdr", Qt::CaseInsensitive))
			{
				HasValidFile = true;
			}
			else if (FileName.contains(".xml", Qt::CaseInsensitive))
			{
				HasValidFile = true;
			}
		}

		if (HasValidFile)
		{
			event->setDropAction(Qt::LinkAction);
			event->accept();
		}
		else
		{
			event->ignore();
		}
	}
	else
	{
		event->ignore();
	}
}

void ARModelPreviewWindow::dragMoveEvent(QDragMoveEvent* event)
{
	event->setDropAction(Qt::LinkAction);
	event->accept();
}

void ARModelPreviewWindow::dragLeaveEvent(QDragLeaveEvent* event)
{
	event->accept();
}

void ARModelPreviewWindow::dropEvent(QDropEvent* event)
{
	const QMimeData* mimeData = event->mimeData();

	if (mimeData->hasUrls())
	{
		QStringList pathList;
		QList<QUrl> urlList = mimeData->urls();

		for (int i = 0; i < urlList.size() ;++i)
		{
			pathList.append(urlList.at(i).toLocalFile());
		}

		for (size_t index = 0; index < pathList.size(); ++index)
		{
			QString FileName = pathList[index];

			if (FileName.contains(".mp4", Qt::CaseInsensitive) || FileName.contains(".mkv", Qt::CaseInsensitive)
				|| FileName.contains(".mov", Qt::CaseInsensitive) || FileName.contains(".flv", Qt::CaseInsensitive) ||
				FileName.contains(".avi", Qt::CaseInsensitive))
			{
				m_BaseToolTab->OpenVideoFile(FileName);
			}
			else if (FileName.contains(".zip", Qt::CaseInsensitive))
			{
				m_BaseToolTab->OpenARZipFile(FileName);
			}
			else if (FileName.contains(".hdr",Qt::CaseInsensitive))
			{
				m_BaseToolTab->OpenHDRFile(FileName);
			}
			else if (FileName.contains(".xml", Qt::CaseInsensitive))
			{
				m_BaseToolTab->OpenXMLFile(FileName);
			}
		}

		event->setDropAction(Qt::LinkAction);
		event->accept();
	}
}

bool ARModelPreviewWindow::event(QEvent* e)
{
	if (QSynchronizeConfigEvent::type == e->type())
	{
		QSynchronizeConfigEvent* InfoE = static_cast<QSynchronizeConfigEvent*>(e);
		m_BaseToolTab->SetUIValue(InfoE);
	}
	else if (QFurDataEvent::type == e->type())
	{
		QFurDataEvent* InfoE = static_cast<QFurDataEvent*>(e);
		m_MaterialToolTab->SetUIValue(InfoE);
	}
	else if (AddSkeletonLayerEvent::type == e->type())
	{
		AddSkeletonLayerEvent* InfoE = static_cast<AddSkeletonLayerEvent*>(e);
		m_BaseToolTab->SetUIValue(InfoE);
	}
	else if (QPostProcessEvent::type == e->type())
	{
		QPostProcessEvent* InfoE = static_cast<QPostProcessEvent*>(e);
		m_PostProcessToolTab->SetUIValue(InfoE);
	}

	return QWidget::event(e);
}
