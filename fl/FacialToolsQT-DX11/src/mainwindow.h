#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QLabel>
#include <QDebug>
#include <QMenuBar>
#include <QMenu>
#include <QPushButton>
#include <QHBoxLayout>
#include"glrenderwiget.h"
#include "common.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
public slots:
  //  void rightWidgetChange(QWidget * w);
    void openCamera();
    void openImage();
    void openVideo();
	void openAudio();
    void openEffect();
    void onSetAlpha(float alpha, CCEffectType type);


private slots:
    void on_MainWindow_iconSizeChanged(const QSize &iconSize);
	//virtual void keyPressEvent(QKeyEvent* e);

private:
    Ui::MainWindow *ui;

    QHBoxLayout*		m_hlayout;
    QVBoxLayout*		m_vlayout;


    QMenuBar*			m_menubar;
    QMenu*				m_menu_file;
    QActionGroup*		m_file_group;
    QAction*			m_openeffect;
    QAction*			m_openimg;
    QAction*			m_opencamera;
    QAction*			m_openvideo;

    QMenu*				m_menu_edit;
    QAction*			m_show_keypoints;
    QAction*			m_show_origin;

    glRenderWiget * m_renderWidget;

	bool enablemakeup=true;
};
#endif // MAINWINDOW_H
