#pragma once

#include <QWidget>
#include <QLabel>
#include <QDebug>
#include <QMenuBar>
#include <QMenu>
#include <QPushButton>
#include <QHBoxLayout>
#include <QEvent>
#include "d3d11renderwidget.h"
#include "layersWidget.h"
#include "EditSwitchWidget.h"
#include "renderBgWidget.h"

class SwitchLayoutEvent : public QEvent
{
public:
	SwitchLayoutEvent();
public:
	static int type;//自定义的事件类型

};

class ARZipPreviewWindow : public QWidget
{
	Q_OBJECT

public:
	ARZipPreviewWindow(QWidget* parent = nullptr);
	~ARZipPreviewWindow();

public slots:
	void rightWidgetChange(QWidget * w);
	void openCamera();
	void openImage();
	void openVideo();
	void openEffect();
	void openARModel();
	void openXML();
	void saveAsZip();
	void PreProcess();
	void openJsonAnimation();

private:
	void change2AR(bool AR);
	virtual bool event(QEvent* e);

private:
	RenderBgWidget*		m_render_bg;
	D3d11RenderWidget*	m_render_widget;
	QHBoxLayout*		m_hlayout;
	QVBoxLayout*		m_vlayout;

	LayersWidget*		m_layer_widget;
	EditSwitchWidget*	m_edit_widget;

	QMenuBar*			m_menubar;
	QMenu*				m_menu_file;
	QActionGroup*		m_file_group;
	QAction*			m_openeffect;
	QAction*			m_openimg;
	QAction*			m_openeJsonAnimation;
	QAction*			m_opencamera;
	QAction*			m_openXML;
	QAction*			m_preprocessVideo;
	QAction*			m_openvideo;
	QAction*			m_save_zip;

	QMenu*				m_menu_edit;
	QAction*			m_show_keypoints;
	QAction*			m_show_origin;
	QAction*			m_add_2DTexture;
	QAction*			m_add_FakeMakeUP;
	QAction*			m_add_FaceEffect3D;

};