#include <QStyleOption>
#include <QPainter>
#include <QAction>
#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QFileDialog>
#include "Scene2DEffect_EditWidget.h"
#include "C2DPicTextureEffect_EditWidget.h"
#include "Toolbox\EffectModel.hpp"

Scene2DEffectEditWidget::Scene2DEffectEditWidget(FacialAnimation *ani, QWidget *parent)
	: m_ani(ani), QWidget(parent){
	//setStyleSheet("background-color:#555555;");

	m_pos = new QLabel(QString::fromLocal8Bit("Î»ÖÃ"), this);
	m_pos_x = new QLineEdit(this);
	m_pos_x->setAlignment(Qt::AlignmentFlag::AlignCenter);
	m_pos_x->setValidator(new QIntValidator);
	m_pos_y = new QLineEdit(this);
	m_pos_y->setAlignment(Qt::AlignmentFlag::AlignCenter);
	m_pos_y->setValidator(new QIntValidator);

	m_hlayout = new QHBoxLayout();
	m_hlayout->addWidget(m_pos);
	m_hlayout->addWidget(m_pos_x);
	m_hlayout->addWidget(m_pos_y);

	m_pos_x->setText(QString::number(m_ani->getEffect2DRectVec()[0].m_x));
	m_pos_y->setText(QString::number(m_ani->getEffect2DRectVec()[0].m_y));
	connect(m_pos_x, &QLineEdit::editingFinished, this, &Scene2DEffectEditWidget::editFinish);
	connect(m_pos_y, &QLineEdit::editingFinished, this, &Scene2DEffectEditWidget::editFinish);

	m_align_box = new QComboBox(this);
	for (auto&& sz : szArrAlignType) {
		m_align_box->addItem(QString::fromStdString(sz));
	}
	int type = m_ani->getEffect2DRectVec()[0].m_nAlignType;
	m_align_box->setCurrentText(QString::fromStdString(getStringFromAlignType(en_AlignParentType(type))));
	connect(m_align_box, static_cast<void(QComboBox::*)(const QString&)>(&QComboBox::activated), this, &Scene2DEffectEditWidget::alignChange);

	m_open_folder = new QLineEdit(this);
	QAction* open = new QAction(this);
	open->setIcon(QApplication::style()->standardIcon(QStyle::SP_DirOpenIcon));
	m_open_folder->setAlignment(Qt::AlignmentFlag::AlignLeft);
	m_open_folder->addAction(open, QLineEdit::TrailingPosition);
	connect(open, &QAction::triggered, this, &Scene2DEffectEditWidget::openFolder);
	connect(m_open_folder, &QLineEdit::returnPressed, this, &Scene2DEffectEditWidget::getImageFileList);

	m_vlayout = new QVBoxLayout;
	m_vlayout->addLayout(m_hlayout);
	m_vlayout->addWidget(m_align_box);
	m_vlayout->addWidget(m_open_folder);

	setLayout(m_vlayout);
}

Scene2DEffectEditWidget::~Scene2DEffectEditWidget() {}

void Scene2DEffectEditWidget::editFinish() {
	if (QObject::sender() == m_pos_x) {
		m_ani->getEffect2DRectVec()[0].m_x = m_pos_x->text().toInt();
		m_ani->getEffect2DRectVec()[0].m_bNeedUpdate = true;
	}else if(QObject::sender() == m_pos_y) {
		m_ani->getEffect2DRectVec()[0].m_y = m_pos_y->text().toInt();
		m_ani->getEffect2DRectVec()[0].m_bNeedUpdate = true;
	}
}

void Scene2DEffectEditWidget::alignChange(const QString & str) {
	en_AlignParentType type = getAlignTypeFromString(str.toStdString());
	m_ani->getEffect2DRectVec()[0].m_nAlignType = type;
	m_ani->getEffect2DRectVec()[0].m_bNeedUpdate = true;
	qDebug() << str;
}

void Scene2DEffectEditWidget::openFolder()
{
	QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"), "./",\
		QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	if (!dir.isEmpty()) {
		m_open_folder->setText(dir);
		getImageFileList();
	}
}

void Scene2DEffectEditWidget::getImageFileList() {
	QString folder = m_open_folder->text();
	QDir dir(folder);
	if (!dir.isEmpty()) {
		std::vector<std::string> img_list;
		QStringList filters = { "*.jpg", "*.png", "*.bmp", "*.jpeg" };
		QFileInfoList file_list = dir.entryInfoList(filters);
		for (auto file_info : file_list) {
			img_list.push_back(file_info.filePath().toStdString());
		}
		m_ani->resetAnidrawable(img_list);
	}
}

void Scene2DEffectEditWidget::paintEvent(QPaintEvent* event) {
	QStyleOption opt;
	opt.init(this);
	QPainter p(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
	QWidget::paintEvent(event);
}
