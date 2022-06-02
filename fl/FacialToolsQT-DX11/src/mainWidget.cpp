#include "mainWidget.h"
#include "effectLayerItem.h"
#include <QFileDialog>
#include <QSizePolicy>

#include "EffectKernel\Sticker\C2DPicTextureEffect.h"
#include "EffectKernel\FaceMakeUp\CFaceMakeUp.h"

MainWidget::MainWidget(QWidget* parent)
	: QWidget(parent) {
	m_render_bg = new RenderBgWidget(this);
	m_render_widget = m_render_bg->getRenderWidget();
	
	m_hlayout = new QHBoxLayout;

	m_edit_widget = new EditSwitchWidget(this);
	//m_edit_widget->setStyleSheet("background-color:#00ff00;");

	m_layer_widget = new LayersWidget(m_render_widget->getSticker(), m_render_widget->getAROrSence(), this);
	connect(m_layer_widget, &LayersWidget::switchWidget, this, &MainWidget::rightWidgetChange);

	m_menu_file = new QMenu("File");
	m_file_group = new QActionGroup(this);
	m_file_group->setExclusive(true);
	m_openeffect = new QAction(QString::fromLocal8Bit("打开AR资源包"));
	m_openeffect->setCheckable(false);
	m_menu_file->addAction(m_openeffect);
	connect(m_openeffect, &QAction::triggered, this, &MainWidget::openEffect);

	m_openeffect = new QAction(QString::fromLocal8Bit("打开3D场景资源包"));
	m_openeffect->setCheckable(false);
	m_menu_file->addAction(m_openeffect);
	connect(m_openeffect, &QAction::triggered, this, &MainWidget::open3DEffect);

	m_save_zip = new QAction(QString::fromLocal8Bit("保存资源"));
	m_save_zip->setCheckable(false);
	m_menu_file->addAction(m_save_zip);
	connect(m_save_zip,  &QAction::triggered, this, &MainWidget::saveAsZip);

	m_openXML = new QAction(QString::fromLocal8Bit("打开XML配置文件"));
	m_openXML->setCheckable(false);
	m_menu_file->addAction(m_openXML);
	connect(m_openXML, &QAction::triggered, this, &MainWidget::openXML);
	m_menu_file->addSeparator();
	
	m_openimg = new QAction("Open Image", m_file_group);
	m_openimg->setCheckable(true);
	m_openimg->setChecked(true);
	m_menu_file->addAction(m_openimg);
	connect(m_openimg, &QAction::triggered, this, &MainWidget::openImage);
	m_opencamera = new QAction("Open Camera", m_file_group);
	m_opencamera->setCheckable(true);
	m_menu_file->addAction(m_opencamera);
	connect(m_opencamera, &QAction::changed, this, &MainWidget::openCamera);
	m_openvideo = new QAction("Open Video", m_file_group);
	m_openvideo->setCheckable(true);
	m_menu_file->addAction(m_openvideo);
	connect(m_openvideo, &QAction::triggered, this, &MainWidget::openVideo);

	
	m_menu_edit = new QMenu("Edit");
	m_show_keypoints = new QAction(QString::fromLocal8Bit("开启人脸检测"));
	m_show_keypoints->setCheckable(true);
	m_show_keypoints->setChecked(m_render_widget->getShowKeyPoints());
	connect(m_show_keypoints, &QAction::toggled, m_render_widget, &D3d11RenderWidget::setShowKeyPoints);
	m_menu_edit->addAction(m_show_keypoints);
	m_show_origin = new QAction(QString::fromLocal8Bit("开启特效"));
	m_show_origin->setCheckable(true);
	m_show_origin->setChecked(m_render_widget->getShowOrigin());
	connect(m_show_origin, &QAction::toggled, m_render_widget, &D3d11RenderWidget::setShowOrigin);
	m_menu_edit->addAction(m_show_origin);

	m_add_2DTexture = new QAction(QString::fromLocal8Bit("开启绿幕抠图"));
	m_add_2DTexture->setCheckable(true);
	m_add_2DTexture->setChecked(false);
	connect(m_add_2DTexture, &QAction::toggled, m_render_widget, &D3d11RenderWidget::setSegment);
	m_menu_edit->addAction(m_add_2DTexture);


// 	m_add_2DTexture = new QAction("Add 2DTexture Effect");
// 	m_add_2DTexture->setCheckable(false);
// 	connect(m_add_2DTexture, &QAction::triggered, this, [&]() {
// 		CEffectPart* part = C2DPicTextureEffect::createEffect();
// 		ccAddEffect(m_render_widget->getSticker(), part);
// 		m_layer_widget->addEffectAR();
// 	});
// 	m_menu_edit->addAction(m_add_2DTexture);


	m_add_FakeMakeUP = new QAction("Add FaceMakeUP");
	m_add_FakeMakeUP->setCheckable(false);
	connect(m_add_FakeMakeUP, &QAction::triggered, this, [&]() {
		CEffectPart* part = CFaceMakeUp::createEffect();
		ccAddEffect(m_render_widget->getSticker(), part);
		m_layer_widget->addEffectAR();
	});
	m_menu_edit->addAction(m_add_FakeMakeUP);
// 	m_add_FaceEffect3D = new QAction(QString::fromLocal8Bit("添加贴片"));
// 	m_add_FaceEffect3D->setCheckable(false);
// 	connect(m_add_FaceEffect3D, &QAction::triggered, this, [&]() {
// 		CEffectPart* part = CFaceEffect3D::createEffect();
// 		ccAddEffect(m_render_widget->getSticker(), part);
// 		m_layer_widget->addEffectAR();
// 	});
// 	m_menu_edit->addAction(m_add_FaceEffect3D);

	m_menubar = new QMenuBar(this);
	m_menubar->addMenu(m_menu_file);
	m_menubar->addMenu(m_menu_edit);

	m_hlayout->addWidget(m_layer_widget);
	m_hlayout->addWidget(m_render_bg, 1);
	m_hlayout->addWidget(m_edit_widget);

	m_vlayout = new QVBoxLayout;
	m_vlayout->addWidget(m_menubar);
	m_vlayout->addLayout(m_hlayout);

	setLayout(m_vlayout);
	change2AR(m_render_widget->getAROrSence());
}

MainWidget::~MainWidget() {}

void MainWidget::openCamera() {
	m_render_widget->changeCameraFileType();
}

void MainWidget::openImage() {
	qDebug() << __func__;
	QString strCombineFilter = tr("Images(*.png *.jpg *.jpeg *.bmp)");

	QString filePath = QFileDialog::getOpenFileName(this, tr("Open Image"), "./", strCombineFilter, nullptr);
	if (!filePath.isEmpty()) {
		m_render_widget->changeImageFileType(filePath);
	}
}

void MainWidget::openVideo(){
	QString strCombineFilter = tr("Video(*.mp4 *.flv *.avi)");
	QString filePath = QFileDialog::getOpenFileName(this, tr("Open Video"), "./", strCombineFilter, nullptr);
	if (!filePath.isEmpty()) {
		m_render_widget->changeVideoFileType(filePath);
	}
}

void MainWidget::openEffect() {
	QString strCombineFilter = tr("Resource(*.zip)");
	QString filePath = QFileDialog::getOpenFileName(this, tr("Open EffectResource"), "./resource", strCombineFilter, nullptr);
	if (!filePath.isEmpty()) {
		m_render_widget->changeEffectResource(filePath);
	}
	if (m_render_widget->getAROrSence()) {
		m_layer_widget->changeStrickerAR(m_render_widget->getSticker());
	} else {
		m_layer_widget->changeStrickerScene(m_render_widget->getSticker());
	}
	change2AR(m_render_widget->getAROrSence());
}


void MainWidget::open3DEffect() {
	QString strCombineFilter = tr("Resource(*.zip)");
	QString filePath = QFileDialog::getOpenFileName(this, tr("Open EffectResource"), "./resource", strCombineFilter, nullptr);
	if (!filePath.isEmpty()) {
		m_render_widget->change3DEffectResource(filePath);
	}
	if (m_render_widget->getAROrSence()) {
		m_layer_widget->changeStrickerAR(m_render_widget->getSticker());
	}
	else {
		m_layer_widget->changeStrickerScene(m_render_widget->getSticker());
	}
	change2AR(m_render_widget->getAROrSence());
}

void MainWidget::openXML()
{
	QString strCombineFilter = tr("Resource(*.xml)");
	QString filePath = QFileDialog::getOpenFileName(this, tr("Open EffectResource"), "./resource", strCombineFilter, nullptr);
	QFileInfo fi = QFileInfo(filePath);
	QString fileName = fi.fileName();
	QString fileDir = fi.dir().absolutePath();
	if (!filePath.isEmpty()) {
		m_render_widget->changeEffectResourceXML(fileDir, fileName);
	}
	if (m_render_widget->getAROrSence()) {
		m_layer_widget->changeStrickerAR(m_render_widget->getSticker());
	}
	else {
		m_layer_widget->changeStrickerScene(m_render_widget->getSticker());
	}
	change2AR(m_render_widget->getAROrSence());
}

void MainWidget::saveAsZip() {
	if (m_render_widget->getAROrSence()) {
		//ccZipAllEffect(m_render_widget->getSticker());
	}
}

void MainWidget::change2AR(bool AR)
{
	m_add_2DTexture->setEnabled(AR);
	m_add_FakeMakeUP->setEnabled(AR);
	//m_add_FaceEffect3D->setEnabled(AR);
}

void MainWidget::rightWidgetChange(QWidget * w) {
	m_edit_widget->switchWidget(w);
}