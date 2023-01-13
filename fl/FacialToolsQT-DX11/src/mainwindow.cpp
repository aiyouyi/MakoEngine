#include<iostream>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QSizePolicy>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), 
	ui(new Ui::MainWindow)
{



	ui->setupUi(this);
	//ui->openGLWidget->makeCurrent();
	m_renderWidget = new glRenderWiget(ui->openGLWidget);
	ui->openGLWidget->setGLwidget(m_renderWidget);
	ui->openGLWidget->initializeGL();

	//QSurfaceFormat format;
	//format.setRenderableType(QSurfaceFormat::OpenGL);
	////format.setProfile(QSurfaceFormat::CoreProfile);
	//format.setProfile(QSurfaceFormat::CompatibilityProfile);
	//format.setVersion(4, 5);
	//ui->openGLWidget->setFormat(format);
	showMaximized();

    //m_menu_file = new QMenu("File");
    //m_file_group = new QActionGroup(this);
    //m_file_group->setExclusive(true);
    //m_openeffect = new QAction(QString::fromLocal8Bit("AR"));
    //m_openeffect->setCheckable(false);
    //m_menu_file->addAction(m_openeffect);
    connect(ui->actionAR, &QAction::triggered, this, &MainWindow::openEffect);

    //m_openimg = new QAction("Open Image", m_file_group);
    //m_openimg->setCheckable(true);
    //m_openimg->setChecked(true);
    //m_menu_file->addAction(m_openimg);
    connect(ui->actionOpenImage, &QAction::triggered, this, &MainWindow::openImage);
    
	//m_opencamera = new QAction("Open Camera", m_file_group);
 //   m_opencamera->setCheckable(true);
 //   m_menu_file->addAction(m_opencamera);
	connect(ui->actionOpenCamera, &QAction::changed, this, &MainWindow::openCamera);
 //   
	//m_openvideo = new QAction("Open Video", m_file_group);
 //   m_openvideo->setCheckable(true);
 //   m_menu_file->addAction(m_openvideo);
    connect(ui->actionOpenVideo, &QAction::triggered, this, &MainWindow::openVideo);

	connect(ui->actionOpenAudio, &QAction::triggered, this, &MainWindow::openAudio);

 //   m_menu_edit = new QMenu("Edit");
 //   m_show_keypoints = new QAction(QString::fromLocal8Bit("face point"));
 //   m_show_keypoints->setCheckable(true);
 //   m_show_keypoints->setChecked(m_renderWidget->getShowKeyPoints());
	//m_menu_edit->addAction(m_show_keypoints);
	connect(ui->actionFacePoint, &QAction::toggled, m_renderWidget, &glRenderWiget::setShowKeyPoints);
 //   
	//m_show_origin = new QAction(QString::fromLocal8Bit("ori"));
 //   m_show_origin->setCheckable(true);
 //   m_show_origin->setChecked(m_renderWidget->getShowOrigin());
	connect(ui->actionOri, &QAction::toggled, m_renderWidget, &glRenderWiget::setShowOrigin);
 //   m_menu_edit->addAction(m_show_origin);

 //   m_menubar = new QMenuBar(this);
 //   m_menubar->addMenu(m_menu_file);
 //   m_menubar->addMenu(m_menu_edit);

    connect(ui->horizontalSlider_InsSmooth, &QSlider::valueChanged, this, [=](int val) {
        this->onSetAlpha(val / 1000.0, SMOOTH_EFFECT);
        ui->lineEdit_InsSmooth->setText(QString::number(val / 1000.0));
    });

    connect(ui->horizontalSlider_LUTFilter, &QSlider::valueChanged, this, [=](int val) {
        this->onSetAlpha(val / 1000.0, LUT_FILTER_EFFECT);
        ui->lineEdit_LUTFilter->setText(QString::number(val / 1000.0));
    });

    connect(ui->horizontalSlider_MouthStick, &QSlider::valueChanged, this, [=](int val) {
        this->onSetAlpha(val / 1000.0, FACE_MOUTH_STRICK);
        ui->lineEdit_MouthStick->setText(QString::number(val / 1000.0));
    });

	connect(ui->comboBox_MouthStick, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [=](int val) {
		m_renderWidget->m_InterFace->setBlendType(val, FACE_MOUTH_STRICK);
	});

    connect(ui->horizontalSlider_EyeOffset, &QSlider::valueChanged, this, [=](int val) {
        this->onSetAlpha(val / 1000.0, FACE_EYE_OFFEST_EFFECT);
        ui->lineEdit_EyeOffset->setText(QString::number(val / 1000.0));
    });

    connect(ui->horizontalSlider_EyeRotate, &QSlider::valueChanged, this, [=](int val) {
        this->onSetAlpha(val / 1000.0, FACE_EYE_ROTATE_EFFECT);
        ui->lineEdit_EyeRotate->setText(QString::number(val / 1000.0));
    });

    connect(ui->horizontalSlider_Forehead, &QSlider::valueChanged, this, [=](int val) {
        this->onSetAlpha(val / 1000.0, FACE_FOREHEAD_EFFECT);
        ui->lineEdit_Forehead->setText(QString::number(val / 1000.0));
    });

    connect(ui->horizontalSlider_MouthPos, &QSlider::valueChanged, this, [=](int val) {
        this->onSetAlpha(val / 1000.0, FACE_MOUTHPOS_EFFECT);
        ui->lineEdit_MouthPos->setText(QString::number(val / 1000.0));
    });

    connect(ui->horizontalSlider_NosePos, &QSlider::valueChanged, this, [=](int val) {
        this->onSetAlpha(val / 1000.0, FACE_NOSEPOS_EFFECT);
        ui->lineEdit_NosePos->setText(QString::number(val / 1000.0));
    });

    connect(ui->horizontalSlider_EyeStick, &QSlider::valueChanged, this, [=](int val) {
        this->onSetAlpha(val / 1000.0, FACE_EYESTRICK);
        ui->lineEdit_EyeStick->setText(QString::number(val / 1000.0));
    });

	connect(ui->comboBox_EyeStick, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [=](int val) {
		m_renderWidget->m_InterFace->setBlendType(val, FACE_EYESTRICK);
	});

    connect(ui->horizontalSlider_Eyelid, &QSlider::valueChanged, this, [=](int val) {
        this->onSetAlpha(val / 1000.0, FACE_EYELID);
        ui->lineEdit_Eyelid->setText(QString::number(val / 1000.0));
    });

	connect(ui->comboBox_Eyelid, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [=](int val) {
		m_renderWidget->m_InterFace->setBlendType(val, FACE_EYELID);
	});

    connect(ui->horizontalSlider_Eyebrow, &QSlider::valueChanged, this, [=](int val) {
        this->onSetAlpha(val / 1000.0, FACE_EYEBROWSTRICK);
        ui->lineEdit_Eyebrow->setText(QString::number(val / 1000.0));
    });

	connect(ui->comboBox_Eyebrow, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [=](int val) {
		m_renderWidget->m_InterFace->setBlendType(val, FACE_EYEBROWSTRICK);
	});

    connect(ui->horizontalSlider_FaceBlush, &QSlider::valueChanged, this, [=](int val) {
        this->onSetAlpha(val / 1000.0, FACE_BLUSH);
        ui->lineEdit_FaceBlush->setText(QString::number(val / 1000.0));
    });

	connect(ui->comboBox_FaceBlush, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [=](int val) {
		m_renderWidget->m_InterFace->setBlendType(val, FACE_BLUSH);
	});

    connect(ui->horizontalSlider_Yamane, &QSlider::valueChanged, this, [=](int val) {
        this->onSetAlpha(val / 1000.0, FACE_YAMANE_EFFECT);
        ui->lineEdit_Yamane->setText(QString::number(val / 1000.0));
    });

    connect(ui->horizontalSlider_FaceHighlight, &QSlider::valueChanged, this, [=](int val) {
        this->onSetAlpha(val / 1000.0, SHADOW_HIGHLIGHT_EFFECT);
        ui->lineEdit_FaceHighlight->setText(QString::number(val / 1000.0));
    });

    connect(ui->horizontalSlider_FaceLevel, &QSlider::valueChanged, this, [=](int val) {
        this->onSetAlpha(val * 1.0, FACE_LEVELS);
        ui->lineEdit_FaceLevel->setText(QString::number(val));
    });

    connect(ui->horizontalSlider_FoodieSharp, &QSlider::valueChanged, this, [=](int val) {
        this->onSetAlpha(val / 1000.0, LUT_CLEAR_EFFECT);
        ui->lineEdit_FoodieSharp->setText(QString::number(val / 1000.0));
    });

    connect(ui->horizontalSlider_FaceLipstick, &QSlider::valueChanged, this, [=](int val) {
        this->onSetAlpha(val / 1000.0, FACE_LIPSTRICK);
        ui->lineEdit_FaceLipstick->setText(QString::number(val / 1000.0));
    });

    connect(ui->horizontalSlider_Eyebig, &QSlider::valueChanged, this, [=](int val) {
        this->onSetAlpha(val / 1000.0, FACE_EYE_BIG_EFFECT);
        ui->lineEdit_Eyebig->setText(QString::number(val / 1000.0));
    });

    connect(ui->horizontalSlider_FaceLift, &QSlider::valueChanged, this, [=](int val) {
        this->onSetAlpha(val / 1000.0, FACE_LIFT_EFFECT);
        ui->lineEdit_FaceLift->setText(QString::number(val / 1000.0));
    });

    connect(ui->horizontalSlider_FaceSmall, &QSlider::valueChanged, this, [=](int val) {
        this->onSetAlpha(val / 1000.0, FACE_SMALL_EFFECT);
        ui->lineEdit_FaceSmall->setText(QString::number(val / 1000.0));
    });

    connect(ui->horizontalSlider_FaceNarrow, &QSlider::valueChanged, this, [=](int val) {
        this->onSetAlpha(val / 1000.0, FACE_NARROW_EFFECT);
        ui->lineEdit_FaceNarrow->setText(QString::number(val / 1000.0));
    });

    connect(ui->horizontalSlider_Chin, &QSlider::valueChanged, this, [=](int val) {
        this->onSetAlpha(val / 1000.0, FACE_CHIN_EFFECT);
        ui->lineEdit_Chin->setText(QString::number(val / 1000.0));
    });

    connect(ui->horizontalSlider_Nose, &QSlider::valueChanged, this, [=](int val) {
        this->onSetAlpha(val / 1000.0, FACE_NOSE_EFFECT);
        ui->lineEdit_Nose->setText(QString::number(val / 1000.0));
    });

    connect(ui->horizontalSlider_Mouth, &QSlider::valueChanged, this, [=](int val) {
        this->onSetAlpha(val / 1000.0, FACE_MOUTH_EFFECT);
        ui->lineEdit_Mouth->setText(QString::number(val / 1000.0));
    });

	connect(ui->horizontalSlider_BrightEye, &QSlider::valueChanged, this, [=](int val) {
		this->onSetAlpha(val / 1000.0, FACE_BRIGHT_EYE);
		ui->lineEdit_BrightEye->setText(QString::number(val / 1000.0));
	});

	connect(ui->horizontalSlider_RemoveFalin, &QSlider::valueChanged, this, [=](int val) {
		this->onSetAlpha(val / 1000.0, FACE_REMOVE_FALIN);
		ui->lineEdit_RemoveFalin->setText(QString::number(val / 1000.0));
	});
	connect(ui->horizontalSlider_RemovePouch, &QSlider::valueChanged, this, [=](int val) {
		this->onSetAlpha(val / 1000.0, FACE_REMOVE_POUCH);
		ui->lineEdit_RemovePouch->setText(QString::number(val / 1000.0));
	});
	connect(ui->horizontalSlider_Checkbone, &QSlider::valueChanged, this, [=](int val) {
		this->onSetAlpha(val / 1000.0, FACE_CHEEK_BONES);
		ui->lineEdit_Checkbone->setText(QString::number(val / 1000.0));
	});
	connect(ui->horizontalSlider_Jaw, &QSlider::valueChanged, this, [=](int val) {
		this->onSetAlpha(val / 1000.0, FACE_LOWER_JAW);
		ui->lineEdit_Jaw->setText(QString::number(val / 1000.0));
	});
	connect(ui->horizontalSlider_Softlight, &QSlider::valueChanged, this, [=](int val) {
		this->onSetAlpha(val / 1000.0, FACE_BLEND_BLUSH);
		ui->lineEdit_Softlight->setText(QString::number(val / 1000.0));
	});

     m_renderWidget->changeVideoFileType("../Resource/2202241433(kiki).mp4");
	// m_renderWidget->changeVideoFileType("../Resource/20201027000.flv");

     //m_renderWidget->changeEffectResource("../Resource/ToukuiBianxing__AR.zip");
	// m_renderWidget->changeEffectResource("../Resource/jiumi02_AR.zip");
     m_renderWidget->changeEffectResource("../Resource/TouShi_AR.zip");
}

MainWindow::~MainWindow()
{
   delete ui;
   delete  m_renderWidget;
}


void MainWindow::openCamera() 
{
    m_renderWidget->changeCamera();
}

void MainWindow::openImage() {
    qDebug() << __func__;
    QString strCombineFilter = tr("Images(*.png *.jpg *.jpeg *.bmp)");

    QString filePath = QFileDialog::getOpenFileName(this, tr("Open Image"), "./", strCombineFilter, nullptr);
    if (!filePath.isEmpty()) {
        m_renderWidget->changeImageFileType(filePath);
    }
}

void MainWindow::openVideo(){
    QString strCombineFilter = tr("Video(*.mp4 *.flv *.avi *.mkv *.mov)");
    QString filePath = QFileDialog::getOpenFileName(this, tr("Open Video"), "./", strCombineFilter, nullptr);
    if (!filePath.isEmpty()) 
	{
		m_renderWidget->changeVideoFileType(filePath);
    }
}

void MainWindow::openAudio()
{
	QString strCombineFilter = tr("Audio(*.wav)");
	QString filePath = QFileDialog::getOpenFileName(this, tr("Open Audio"), "./", strCombineFilter, nullptr);
	if (!filePath.isEmpty())
	{
		//m_renderWidget->changeAudioFileType(filePath);
	}
}

void MainWindow::openEffect() {
    QString strCombineFilter = tr("Resource(*.zip)");
    QString filePath = QFileDialog::getOpenFileName(this, tr("Open EffectResource"), "../Resource/", strCombineFilter, nullptr);
    if (!filePath.isEmpty()) {
        m_renderWidget->changeEffectResource(filePath);
    }
}

void MainWindow::onSetAlpha(float alpha, CCEffectType type)
{
    m_renderWidget->m_InterFace->setAlpha(alpha, type);
}

void MainWindow::on_MainWindow_iconSizeChanged(const QSize &iconSize)
{

}
