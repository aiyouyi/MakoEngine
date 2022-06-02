#include <QStyleOption>
#include <QPainter>
#include <QAction>
#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QFileDialog>
#include "C2DPicTextureEffect_EditWidget.h"
#include "EffectKernel\Sticker\C2DPicTextureEffect.h"


C2DPicTextEidtWidget::C2DPicTextEidtWidget(C2DPicTextureEffect *pic, QWidget *parent)
	: m_pic(pic), QWidget(parent){
	//setStyleSheet("background-color:#555555;");

// 	m_pos = new QLabel(QString::fromLocal8Bit("Î»ÖÃ"), this);
// 	m_pos_x = new QLineEdit(this);
// 	m_pos_x->setAlignment(Qt::AlignmentFlag::AlignCenter);
// 	m_pos_x->setValidator(new QIntValidator);
// 	m_pos_y = new QLineEdit(this);
// 	m_pos_y->setAlignment(Qt::AlignmentFlag::AlignCenter);
// 	m_pos_y->setValidator(new QIntValidator);
// 	m_hlayout = new QHBoxLayout();
// 	m_hlayout->addWidget(m_pos);
// 	m_hlayout->addWidget(m_pos_x);
// 	m_hlayout->addWidget(m_pos_y);
// 
// 	m_width_label = new QLabel(QString::fromLocal8Bit("¿í"), this);
// 	m_width = new QLineEdit(this);
// 	m_width->setAlignment(Qt::AlignmentFlag::AlignCenter);
// 	m_width->setValidator(new QIntValidator);
// 	m_height_label = new QLabel(QString::fromLocal8Bit("¸ß"), this);
// 	m_height = new QLineEdit(this);
// 	m_height->setAlignment(Qt::AlignmentFlag::AlignCenter);
// 	m_height->setValidator(new QIntValidator);
// 	QHBoxLayout* hh = new QHBoxLayout();
// 	hh->addWidget(m_width_label);
// 	hh->addWidget(m_width);
// 	hh->addWidget(m_height_label);
// 	hh->addWidget(m_height);
// 
// 	m_pos_x->setText(QString::number(m_pic->getEffect2DRectVec().m_x));
// 	m_pos_y->setText(QString::number(m_pic->getEffect2DRectVec().m_y));
// 	m_width->setText(QString::number(m_pic->getEffect2DRectVec().m_width));
// 	m_height->setText(QString::number(m_pic->getEffect2DRectVec().m_height));
// 	connect(m_pos_x, &QLineEdit::editingFinished, this, &C2DPicTextEidtWidget::editFinish);
// 	connect(m_pos_y, &QLineEdit::editingFinished, this, &C2DPicTextEidtWidget::editFinish);
// 	connect(m_width, &QLineEdit::editingFinished, this, &C2DPicTextEidtWidget::editFinish);
// 	connect(m_height, &QLineEdit::editingFinished, this, &C2DPicTextEidtWidget::editFinish);
// 
// 	m_align_box = new QComboBox(this);
// 	for (auto&& sz : szArrAlignType) {
// 		m_align_box->addItem(QString::fromStdString(sz));
// 	}
// 	int type = m_pic->getEffect2DRectVec().m_nAlignType;
// 	m_align_box->setCurrentText(QString::fromStdString(getStringFromAlignType(en_AlignParentType(type))));
// 	connect(m_align_box, static_cast<void(QComboBox::*)(const QString&)>(&QComboBox::activated), this, &C2DPicTextEidtWidget::alignChange);
// 
// 	m_open_folder = new QLineEdit(this);
// 	QAction* open = new QAction(this);
// 	open->setIcon(QApplication::style()->standardIcon(QStyle::SP_DirOpenIcon));
// 	m_open_folder->setAlignment(Qt::AlignmentFlag::AlignLeft);
// 	m_open_folder->addAction(open, QLineEdit::TrailingPosition);
// 	connect(open, &QAction::triggered, this, &C2DPicTextEidtWidget::openFolder);
// 	connect(m_open_folder, &QLineEdit::returnPressed, this, &C2DPicTextEidtWidget::getImageFileList);
// 
// 	m_vlayout = new QVBoxLayout;
// 	m_vlayout->addLayout(m_hlayout);
// 	m_vlayout->addLayout(hh);
// 	m_vlayout->addWidget(m_align_box);
// 	m_vlayout->addWidget(m_open_folder);
// 	m_vlayout->addStretch(1);
// 
// 	setLayout(m_vlayout);
}

C2DPicTextEidtWidget::~C2DPicTextEidtWidget() {}

void C2DPicTextEidtWidget::editFinish() {
// 	if (QObject::sender() == m_pos_x) {
// 		m_pic->getEffect2DRectVec().m_x = m_pos_x->text().toInt();
// 		m_pic->getEffect2DRectVec().m_bNeedUpdate = true;
// 	}else if(QObject::sender() == m_pos_y) {
// 		m_pic->getEffect2DRectVec().m_y = m_pos_y->text().toInt();
// 		m_pic->getEffect2DRectVec().m_bNeedUpdate = true;
// 	}else if(QObject::sender() == m_width) {
// 		m_pic->getEffect2DRectVec().m_width = m_width->text().toInt();
// 		m_pic->getEffect2DRectVec().m_bNeedUpdate = true;
// 	}else if(QObject::sender() == m_height) {
// 		m_pic->getEffect2DRectVec().m_height = m_height->text().toInt();
// 		m_pic->getEffect2DRectVec().m_bNeedUpdate = true;
// 	}
}

void C2DPicTextEidtWidget::alignChange(const QString & str) {
// 	en_AlignParentType type = getAlignTypeFromString(str.toStdString());
// 	m_pic->getEffect2DRectVec().m_nAlignType = type;
// 	m_pic->getEffect2DRectVec().m_bNeedUpdate = true;
// 	qDebug() << str;
}

void C2DPicTextEidtWidget::openFolder()
{
// 	QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"), "./",\
// 		QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
// 	if (!dir.isEmpty()) {
// 		m_open_folder->setText(dir);
// 		getImageFileList();
// 	}
}

void C2DPicTextEidtWidget::getImageFileList() {
// 	QString folder = m_open_folder->text();
// 	QDir dir(folder);
// 	if (!dir.isEmpty()) {
// 		std::vector<std::string> img_list;
// 		QStringList filters = { "*.jpg", "*.png", "*.bmp", "*.jpeg" };
// 		QFileInfoList file_list = dir.entryInfoList(filters);
// 		for (auto file_info : file_list) {
// 			img_list.push_back(file_info.filePath().toStdString());
// 		}
// 		m_pic->resetAnidrawable(img_list);
// 	}
}

void C2DPicTextEidtWidget::paintEvent(QPaintEvent* event) {
// 	QStyleOption opt;
// 	opt.init(this);
// 	QPainter p(this);
// 	style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
// 	QWidget::paintEvent(event);
}
