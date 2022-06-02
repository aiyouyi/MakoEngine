#include "effectLayerItem.h"
#include <QDebug>
#include <QMouseEvent>

EffectLayerItem::EffectLayerItem(QString name, QWidget* parent, bool AR)
	: QWidget(parent) {
	m_box = new QCheckBox(this);
	m_name = new QLabel(name, this);
	m_layout = new QHBoxLayout;
	if(AR) {
		m_delete_btn = new QPushButton(QString::fromLocal8Bit("删除"), this);
		connect(m_delete_btn, &QPushButton::pressed, this, [this](){ emit deleted(this); });
	}
	m_AR = AR;
	//转发m_name这个对象的所有的鼠标事件
	m_name->setAttribute(Qt::WA_TransparentForMouseEvents, true);

	m_layout->setSpacing(0);
	m_layout->setContentsMargins(0, 0, 0, 0);
	m_layout->addWidget(m_box);
	m_layout->addSpacing(10);
	m_layout->addWidget(m_name);
	m_layout->addStretch(1);
	if(AR)	m_layout->addWidget(m_delete_btn);

	setLayout(m_layout);

	setFixedHeight(ItemHeight);
}

EffectLayerItem::~EffectLayerItem() {
}

void EffectLayerItem::mousePressEvent(QMouseEvent * event){
	qDebug() << "effect item click";
	emit clicked(this);
	if (m_AR)
	{
		event->ignore();
	}
	//
}
