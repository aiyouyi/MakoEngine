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
		msg.setText(QString::fromStdWString(L"ѡ����ʾģʽ"));
		msg.setIcon(QMessageBox::Question);//����ͼ������
		msg.setStandardButtons(QMessageBox::Yes | QMessageBox::No);//�Ի����ϰ����İ�ť
		msg.setButtonText(QMessageBox::Yes, QString::fromStdWString(L"��ƵARZIPģʽ"));
		msg.setButtonText(QMessageBox::No, QString::fromStdWString(L"GLBģ��ģʽ"));

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
