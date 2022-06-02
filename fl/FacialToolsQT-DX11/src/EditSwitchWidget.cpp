#include "EditSwitchWidget.h"
#include <QStyleOption>
#include <QPainter>

EditSwitchWidget::EditSwitchWidget(QWidget* parent)
	: QWidget(parent) {
	setFixedWidth(200);
	m_edit_widget = nullptr;
	m_layout = new QVBoxLayout;
	m_layout->setContentsMargins(0, 0, 0, 0);
	m_layout->setSpacing(0);
	m_layout->setMargin(0);

	setLayout(m_layout);
}

EditSwitchWidget::~EditSwitchWidget() {
}

void EditSwitchWidget::switchWidget(QWidget * w)
{
	if (m_edit_widget) {
		m_edit_widget->setHidden(true);
		m_layout->removeWidget(m_edit_widget);
	}
	m_edit_widget = w;
	if (m_edit_widget) {
		m_edit_widget->setHidden(false);
		m_layout->insertWidget(0, m_edit_widget);
	}
}

void EditSwitchWidget::paintEvent(QPaintEvent * event) {
	QStyleOption opt;
	opt.init(this);
	QPainter p(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
	QWidget::paintEvent(event);
}
