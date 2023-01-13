#include <QStyleOption>
#include <QPainter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QResizeEvent>
#include <QSpacerItem>
#include <QDebug>
#include "renderBgWidget.h"

RenderBgWidget::RenderBgWidget(QWidget* parent)
	: QWidget(parent) {
	setStyleSheet("background-color:#000000;");
	m_render = new D3d11RenderWidget(this);
	m_render->Init();
	m_layout = new QVBoxLayout;
	m_layout->setContentsMargins(0, 0, 0, 0);
	m_layout->addWidget(m_render);
	
	setLayout(m_layout);
}

RenderBgWidget::~RenderBgWidget() {}

void RenderBgWidget::paintEvent(QPaintEvent * event) {
	QStyleOption opt;
	opt.init(this);
	QPainter p(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
	QWidget::paintEvent(event);
}
