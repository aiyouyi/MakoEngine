#include "Light_EditWidget.h"

#include <QStyleOption>
#include <QPainter>
#include <QDebug>
#include <QItemDelegate>
#include <QLineEdit>
#include <QHeaderView>

LightEditWidget::LightEditWidget(FacialAnimation* ani, QWidget* parent)
	: m_ani(ani), QWidget(parent) {
	//setStyleSheet("background-color:#ff00ff;");

	m_vlayout = new QVBoxLayout;
	addClusters();
	m_vlayout->addStretch(1);

	setLayout(m_vlayout);
}

LightEditWidget::~LightEditWidget() {
	for (auto c : m_cluster) {
		delete c;
	}
	m_cluster.clear();
}

void LightEditWidget::addClusters() {
	vector<BaseLight>& lights = m_ani->getLight();
	
	for (int i = 0; i < lights.size(); i++) {
		EditCluster *ec = new EditCluster();
		ec->m_vlayout = new QVBoxLayout;
		{
			ec->m_orientation = new QLabel(QString::fromLocal8Bit("光方向:"), this);

			ec->m_orientation_x = new QLineEdit(this);
			ec->m_orientation_x->setAlignment(Qt::AlignmentFlag::AlignCenter);
			ec->m_orientation_x->setValidator(new QDoubleValidator(-1.f, 1.f, 2));
			ec->m_orientation_x->setText(QString::number(lights[i].m_vOrientation.x, 'g', 2));
			connect(ec->m_orientation_x, &QLineEdit::editingFinished, this, [this, ec, i]() { this->editChange(ec, i); });

			ec->m_orientation_y = new QLineEdit(this);
			ec->m_orientation_y->setAlignment(Qt::AlignmentFlag::AlignCenter);
			ec->m_orientation_y->setValidator(new QDoubleValidator(-1.f, 1.f, 2));
			ec->m_orientation_y->setText(QString::number(lights[i].m_vOrientation.y, 'g', 2));
			connect(ec->m_orientation_y, &QLineEdit::editingFinished, this, [this, ec, i]() { this->editChange(ec, i); });

			ec->m_orientation_z = new QLineEdit(this);
			ec->m_orientation_z->setAlignment(Qt::AlignmentFlag::AlignCenter);
			ec->m_orientation_z->setValidator(new QDoubleValidator(-1.f, 1.f, 2));
			ec->m_orientation_z->setText(QString::number(lights[i].m_vOrientation.z, 'g', 2));
			connect(ec->m_orientation_z, &QLineEdit::editingFinished, this, [this, ec, i]() { this->editChange(ec, i); });

			QHBoxLayout* hl = new QHBoxLayout;
			hl->addWidget(ec->m_orientation);
			hl->addWidget(ec->m_orientation_x);
			hl->addWidget(ec->m_orientation_y);
			hl->addWidget(ec->m_orientation_z);
			ec->m_vlayout->addLayout(hl);
		}
		{
			ec->m_ambient = new QLabel(QString::fromLocal8Bit("环境光:"), this);
			ec->m_ambient_w = new QLineEdit(this);
			ec->m_ambient_w->setAlignment(Qt::AlignmentFlag::AlignCenter);
			ec->m_ambient_w->setValidator(new QDoubleValidator(0.f, 2.f, 2));
			ec->m_ambient_w->setText(QString::number(lights[i].m_vAmbient.x, 'g', 2));
			connect(ec->m_ambient_w, &QLineEdit::editingFinished, this, [this, ec, i]() { this->editChange(ec, i); });

			QHBoxLayout* hl = new QHBoxLayout;
			hl->addWidget(ec->m_ambient);
			hl->addWidget(ec->m_ambient_w);
			ec->m_vlayout->addLayout(hl);
		}
		{
			ec->m_diffuse = new QLabel(QString::fromLocal8Bit("漫反射:"), this);
			ec->m_diffuse_w = new QLineEdit(this);
			ec->m_diffuse_w->setAlignment(Qt::AlignmentFlag::AlignCenter);
			ec->m_diffuse_w->setValidator(new QDoubleValidator(0.f, 2.f, 2));
			ec->m_diffuse_w->setText(QString::number(lights[i].m_vDiffuse.x, 'g', 2));
			connect(ec->m_diffuse_w, &QLineEdit::editingFinished, this, [this, ec, i]() { this->editChange(ec, i); });

			QHBoxLayout* hl = new QHBoxLayout;
			hl->addWidget(ec->m_diffuse);
			hl->addWidget(ec->m_diffuse_w);
			ec->m_vlayout->addLayout(hl);
		}
		{
			ec->m_specular = new QLabel(QString::fromLocal8Bit("镜面反射:"), this);
			ec->m_specular_w = new QLineEdit(this);
			ec->m_specular_w->setAlignment(Qt::AlignmentFlag::AlignCenter);
			ec->m_specular_w->setValidator(new QDoubleValidator(0.f, 2.f, 2));
			ec->m_specular_w->setText(QString::number(lights[i].m_vSpecular.x, 'g', 2));
			connect(ec->m_specular_w, &QLineEdit::editingFinished, this, [this, ec, i]() { this->editChange(ec, i); });

			QHBoxLayout* hl = new QHBoxLayout;
			hl->addWidget(ec->m_specular);
			hl->addWidget(ec->m_specular_w);
			ec->m_vlayout->addLayout(hl);
		}

		m_vlayout->addLayout(ec->m_vlayout);
		m_cluster.push_back(ec);
	}
}


void LightEditWidget::editChange(EditCluster* ec, int index) {
	vector<BaseLight>& lights = m_ani->getLight();
	if (index < lights.size()) {
		qDebug() << __func__;
		QLineEdit* le = static_cast<QLineEdit*>(QObject::sender());
		if (le == ec->m_orientation_x) {
			lights[index].m_vOrientation.x = le->text().toFloat();
		} else if (le == ec->m_orientation_y) {
			lights[index].m_vOrientation.y = le->text().toFloat();
		} else if (le == ec->m_orientation_z) {
			lights[index].m_vOrientation.z = le->text().toFloat();
		} else if (le == ec->m_ambient_w) {
			lights[index].m_vAmbient.x = le->text().toFloat();
			lights[index].m_vAmbient.y = le->text().toFloat();
			lights[index].m_vAmbient.z = le->text().toFloat();
			lights[index].m_vAmbient.w = le->text().toFloat();
		} else if (le == ec->m_diffuse_w) {
			lights[index].m_vDiffuse.x = le->text().toFloat();
			lights[index].m_vDiffuse.y = le->text().toFloat();
			lights[index].m_vDiffuse.z = le->text().toFloat();
			lights[index].m_vDiffuse.w = le->text().toFloat();
		} else if (le == ec->m_specular_w) {
			lights[index].m_vSpecular.x = le->text().toFloat();
			lights[index].m_vSpecular.y = le->text().toFloat();
			lights[index].m_vSpecular.z = le->text().toFloat();
			lights[index].m_vSpecular.w = le->text().toFloat();
		} 
	}
}

void LightEditWidget::paintEvent(QPaintEvent* event) {
	QStyleOption opt;
	opt.init(this);
	QPainter p(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
	QWidget::paintEvent(event);
}
