#include <QStyleOption>
#include <QPainter>
#include "FacialEditWidget.h"


FacialAnimation *g_ani = NULL;
void FacialAnimationCallback(int event, const char *szID, const void *lpParam)
{
	if ((event == AE_REPEAT || event == AE_END) && strcmp(szID, "idle") != 0)
	{
		g_ani->play("idle");
	}
}

FacialEditWidget::FacialEditWidget(FacialAnimation *ani, FacialObject* obj, QWidget* parent)
	: m_ani(ani), m_face_obj(obj), QWidget(parent) {
	//setStyleSheet("background-color:#005555;");
	g_ani = ani;
	m_vlayout = new QVBoxLayout;

	m_pos.init(QString::fromLocal8Bit("Î»ÖÃ"), m_face_obj->getPosition3D(), std::bind(&FSObject::setPosition3D, m_face_obj, std::placeholders::_1), this);
	EditFinishFun setScale = [obj](cocos2d::Vec3 v) { obj->setScaleX(v.x); obj->setScaleY(v.y); obj->setScaleZ(v.z); };
	m_scale.init(QString::fromLocal8Bit("·ÅËõ"), cocos2d::Vec3(m_face_obj->getScaleX(), m_face_obj->getScaleY(), m_face_obj->getScaleZ()), \
		setScale, this);
	m_rotate.init(QString::fromLocal8Bit("Ðý×ª"), m_face_obj->getRotation3D(), std::bind(&FSObject::setRotation3D, m_face_obj, std::placeholders::_1), this);
	m_vlayout->addLayout(m_pos.m_layout);
	m_vlayout->addLayout(m_scale.m_layout);
	m_vlayout->addLayout(m_rotate.m_layout);

	std::vector<string> keys;
	m_face_obj->getAnimationKeys(keys);
	for (auto&& key : keys) {
		cocos2d::Action* act = m_face_obj->getAnimationAction(key);
		if (act) {
			AnimateEdit* ani_edit = new AnimateEdit(key, act);
			m_animates.push_back(ani_edit);
			connect(ani_edit, &AnimateEdit::press, this, [this](std::string id){ this->m_ani->play(id.c_str(), FacialAnimationCallback,NULL); });
			connect(ani_edit, &AnimateEdit::deleteSignal, this, &FacialEditWidget::deleteAnimate);
		}
	}
	for (auto&& ani_edit : m_animates) {
		m_vlayout->addWidget(ani_edit);
	}
	m_add_animate = new AddAnimate();
	connect(m_add_animate, &AddAnimate::press, this, &FacialEditWidget::addAnimte);
	m_vlayout->addWidget(m_add_animate);

	m_vlayout->addStretch(1);

	setLayout(m_vlayout);
}

FacialEditWidget::~FacialEditWidget() { }

void FacialEditWidget::paintEvent(QPaintEvent* event) {
	QStyleOption opt;
	opt.init(this);
	QPainter p(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
	QWidget::paintEvent(event);
}

void FacialEditWidget::addAnimte(const string id, const string mode) {
	cocos2d::Action* act = m_ani->addAnimation(id, mode, m_face_obj);
	if (act) {
		AnimateEdit* ani_edit = new AnimateEdit(id, act);
		m_animates.push_back(ani_edit);
		connect(ani_edit, &AnimateEdit::press, this, [this](std::string id){ this->m_ani->play(id.c_str(), FacialAnimationCallback); });
		connect(ani_edit, &AnimateEdit::deleteSignal, this, &FacialEditWidget::deleteAnimate);
		int count = m_vlayout->count();
		m_vlayout->insertWidget(count - 2, ani_edit);
	}
}

void FacialEditWidget::deleteAnimate(AnimateEdit* w, std::string id) {
	auto fw = std::find(m_animates.begin(), m_animates.end(), w);
	if (fw != m_animates.end()) {
		m_animates.erase(fw);
		m_vlayout->removeWidget(w);
		m_face_obj->eraseAnimation(id);
		delete w;
	}
}