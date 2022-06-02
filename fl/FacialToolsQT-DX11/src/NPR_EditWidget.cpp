#include "NPR_EditWidget.h"

#include <QStyleOption>
#include <QPainter>
#include <QDebug>

NPREditWidget::NPREditWidget(FacialAnimation* ani, QWidget* parent)
	: m_ani(ani), QWidget(parent) {
	//setStyleSheet("background-color:#5555ff;");
	
	m_vlayout = new QVBoxLayout;
	NPRInfo& npr = m_ani->getNPR();

	m_use_npr = new QCheckBox(QString::fromLocal8Bit("NPR"), this);
	m_use_npr->setCheckable(true);
	m_use_npr->setChecked(npr.m_bUseNPR);
	connect(m_use_npr, &QCheckBox::toggled, this, [this](bool checked){ this->m_ani->getNPR().m_bUseNPR = checked; });
	m_vlayout->addWidget(m_use_npr);

	m_line_label = new QLabel(QString::fromLocal8Bit("线宽[0, 0.5]"), this);
	m_line_width = new QLineEdit(this);
	m_line_width->setValidator(new QDoubleValidator(0.f, 0.5f, 2));
	m_line_width->setText(QString::number(npr.m_fLineWidth));
	connect(m_line_width, &QLineEdit::editingFinished, this, [&]() { m_ani->getNPR().m_fLineWidth = m_line_width->text().toFloat();	});
	QHBoxLayout* hlayout = new QHBoxLayout;
	hlayout->addWidget(m_line_label);
	hlayout->addWidget(m_line_width);
	m_vlayout->addLayout(hlayout);

	m_color_btn = new QPushButton(QString::fromLocal8Bit("线的颜色"), this);
	QColor cl;
	cl.setRgbF(npr.m_vLineColor.x, npr.m_vLineColor.y, npr.m_vLineColor.z, npr.m_vLineColor.w);
	m_color_btn->setStyleSheet("background: " + cl.name());
	connect(m_color_btn, &QPushButton::clicked, this, &NPREditWidget::selectColor);
	m_vlayout->addWidget(m_color_btn);

	m_vlayout->addStretch(1);
	setLayout(m_vlayout);
}

NPREditWidget::~NPREditWidget() {}

void NPREditWidget::paintEvent(QPaintEvent* event) {
	QStyleOption opt;
	opt.init(this);
	QPainter p(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
	QWidget::paintEvent(event);
}

void NPREditWidget::selectColor() {
	NPRInfo& npr = m_ani->getNPR();
	QColor cl;
	cl.setRgbF(npr.m_vLineColor.x, npr.m_vLineColor.y, npr.m_vLineColor.z, npr.m_vLineColor.w);
	QColorDialog *color_dialog = new QColorDialog();
    color_dialog->setCurrentColor(cl);
    connect(color_dialog, &QColorDialog::currentColorChanged, this, &NPREditWidget::changeColor);
	connect(color_dialog, &QColorDialog::rejected, this, [this, cl](){ this->changeColor(cl);});
	color_dialog->exec();

	delete color_dialog;
}

void NPREditWidget::changeColor(QColor color) {
	NPRInfo& npr = m_ani->getNPR();

	npr.m_vLineColor.x = color.redF();
	npr.m_vLineColor.y = color.greenF();
	npr.m_vLineColor.z = color.blueF();
	npr.m_vLineColor.w = color.alphaF();

	m_color_btn->setStyleSheet("background: " + color.name());
}