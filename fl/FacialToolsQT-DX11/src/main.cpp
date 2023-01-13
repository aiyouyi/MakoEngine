#include <QtWidgets/QApplication>
#include <QDebug>
#include <QScreen>
#include <QMessageBox>
#include "ARZipPreviewWindow.h"
#include "mainwindow.h"
#include "3DModelPreview/ARModelPreviewWindow.h"
#include "Toolbox/Render/DynamicRHI.h"

const bool g_useOpengl = false;

int main(int argc, char *argv[])
{
	SetConsoleOutputCP(65001);
	QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
	QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

	QApplication App(argc, argv);

	
	std::shared_ptr<QWidget> MainWnd = nullptr;
	if (g_useOpengl)
	{
		gRenderAPI = CC3DDynamicRHI::OPENGL;
		MainWnd = std::make_shared<MainWindow>();
		MainWnd->show();
		return App.exec();
	}
	else
	{
		QMessageBox msg(nullptr);
		msg.setText(QString::fromStdWString(L"选择显示模式"));
		msg.setIcon(QMessageBox::Question);//设置图标类型
		msg.setStandardButtons(QMessageBox::Yes | QMessageBox::No);//对话框上包含的按钮
		msg.setButtonText(QMessageBox::Yes, QString::fromStdWString(L"视频ARZIP模式"));
		msg.setButtonText(QMessageBox::No, QString::fromStdWString(L"GLB模型模式"));

		if (msg.exec() == QMessageBox::Yes)
		{
			QScreen* screen = QGuiApplication::primaryScreen();
			QRect screenRect = screen->geometry();

			gRenderAPI = CC3DDynamicRHI::DX11;
			MainWnd = std::make_shared<ARZipPreviewWindow>();
			MainWnd->resize(1920, 1080);
			MainWnd->move(QPoint((screenRect.width() - 1920) / 2, (screenRect.height() - 1080) / 2));
			MainWnd->show();
		}
		else
		{
			MainWnd = std::make_shared<ARModelPreviewWindow>(nullptr);
			
			MainWnd->show();
			(std::static_pointer_cast<ARModelPreviewWindow>(MainWnd))->StartPreview();
			
		}

		return App.exec();
	}

}
