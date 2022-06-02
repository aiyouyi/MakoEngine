#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QDoubleValidator>
#include <functional>
#include <QComboBox>
#include <QMessageBox>
#include <QDebug>
#include "EffectKernel2\FacialAnimation.h"

using EditFinishFun = std::function<void (cocos2d::Vec3)>;
struct EditVec3 : public QObject {
	QLabel*			m_label;
	QLineEdit*		m_x;
	QLineEdit*		m_y;
	QLineEdit*		m_z;
	QHBoxLayout*	m_layout;

	EditFinishFun	m_editfinish;

	void valueChange() {
		float x = m_x->text().toFloat();
		float y = m_y->text().toFloat();
		float z = m_z->text().toFloat();
		m_editfinish(cocos2d::Vec3(x, y, z));
	}

	void init(QString label, cocos2d::Vec3 v, EditFinishFun f, QWidget* parent = nullptr) {
		m_label = new QLabel(label, parent);
		
		m_x = new QLineEdit(parent);
		m_x->setValidator(new QDoubleValidator);
		m_x->setText(QString::number(v.x));
		m_x->setAlignment(Qt::AlignCenter);
		QObject::connect(m_x, &QLineEdit::editingFinished, this, &EditVec3::valueChange);

		m_y = new QLineEdit(parent);
		m_y->setValidator(new QDoubleValidator);
		m_y->setText(QString::number(v.y));
		m_y->setAlignment(Qt::AlignCenter);
		QObject::connect(m_y, &QLineEdit::editingFinished, this, &EditVec3::valueChange);

		m_z = new QLineEdit(parent);
		m_z->setValidator(new QDoubleValidator);
		m_z->setText(QString::number(v.z));
		m_z->setAlignment(Qt::AlignCenter);
		QObject::connect(m_z, &QLineEdit::editingFinished, this, &EditVec3::valueChange);

		m_editfinish = f;

		m_layout = new QHBoxLayout;
		m_layout->addWidget(m_label);
		m_layout->addWidget(m_x);
		m_layout->addWidget(m_y);
		m_layout->addWidget(m_z);
	}
};

class AnimateEdit : public QWidget {
	Q_OBJECT
public:
	AnimateEdit(std::string id, cocos2d::Action* action) {
		m_idkey = id;
		m_action = action;
		m_layout = new QVBoxLayout;
		{
			m_id_label = new QLabel(QString::fromLocal8Bit("ID"), this);
			m_id = new QLineEdit(this);
			m_id->setEnabled(false);
			m_id->setText(QString::fromStdString(m_idkey));
			m_id->setAlignment(Qt::AlignCenter);

			m_delete = new QPushButton(QString::fromLocal8Bit("删除"), this);
			connect(m_delete, &QPushButton::pressed, this, [this]() { emit deleteSignal(this, this->m_idkey);});
		
			QHBoxLayout* hl = new QHBoxLayout;
			hl->addWidget(m_id_label);
			hl->addWidget(m_id);
			hl->addWidget(m_delete);
			m_layout->addLayout(hl);
		}
		{
			cocos2d::Animate3D* act3d = dynamic_cast<cocos2d::Animate3D*>(action);
			if (act3d == nullptr) {
				cocos2d::RepeatForever * re = dynamic_cast<cocos2d::RepeatForever*>(action);
				if(re) act3d = dynamic_cast<cocos2d::Animate3D*>(re->getAction());
			}
			if(act3d) {
				m_start_label = new QLabel(QString::fromLocal8Bit("开始"), this);
				m_start = new QLineEdit(this);
				m_start->setText(QString::number(act3d->getStartFrame()));
				m_start->setAlignment(Qt::AlignCenter);
				connect(m_start, &QLineEdit::editingFinished, this, [act3d, this](){ act3d->setStartFrame(this->m_start->text().toInt()); });

				m_end_label = new QLabel(QString::fromLocal8Bit("结束"), this);
				m_end = new QLineEdit(this);
				m_end->setText(QString::number(act3d->getEndFrame()));
				m_end->setAlignment(Qt::AlignCenter);
				connect(m_end, &QLineEdit::editingFinished, this, [act3d, this](){ act3d->setEndFrame(this->m_end->text().toInt()); });

				QHBoxLayout* hl = new QHBoxLayout;
				hl->addWidget(m_start_label);
				hl->addWidget(m_start);
				hl->addWidget(m_end_label);
				hl->addWidget(m_end);
				m_layout->addLayout(hl);
			}
		}
		m_play = new QPushButton(QString::fromLocal8Bit("播放"), this);
		m_layout->addWidget(m_play);

		connect(m_play, &QPushButton::pressed, this, [this](){ emit press(this->m_idkey);});

		setLayout(m_layout);
	}

Q_SIGNALS:
	void press(std::string id);
	void deleteSignal(AnimateEdit* w, std::string id);

private:
	QLabel*			m_id_label;
	QLineEdit*		m_id;
	QLabel*			m_start_label;
	QLineEdit*		m_start;
	QLabel*			m_end_label;
	QLineEdit*		m_end;
	QPushButton*	m_play;
	QPushButton*	m_delete;

	QVBoxLayout*	 m_layout;
	
	std::string		 m_idkey;
	cocos2d::Action* m_action;
};

class AddAnimate : public QWidget {
	Q_OBJECT
public:
	AddAnimate() {
		m_id_label = new QLabel(QString::fromLocal8Bit("ID"), this);
		m_id = new QLineEdit(this);
		m_id->setAlignment(Qt::AlignCenter);
		m_mode = new QComboBox(this);
		for (auto&& sz : {"repeat", "oneshot"}) {
			m_mode->addItem(sz);
		}
		m_mode->setCurrentText("oneshot");
		m_add = new QPushButton(QString::fromLocal8Bit("添加"));
		connect(m_add, &QPushButton::pressed, this, [this]() {
			if ("" == m_id->text()) {
				QMessageBox::about(nullptr,  QString::fromLocal8Bit("提示"), QString::fromLocal8Bit("请输入ID"));
			}else{
				emit press(m_id->text().toStdString(), m_mode->currentText().toStdString());
			}
		});

		QHBoxLayout* m_hlayout = new QHBoxLayout;
		m_hlayout->addWidget(m_id_label);
		m_hlayout->addWidget(m_id);
		m_hlayout->addWidget(m_mode);
		m_vlayout = new QVBoxLayout;
		m_vlayout->addLayout(m_hlayout);
		m_vlayout->addWidget(m_add);

		setLayout(m_vlayout);
	}

Q_SIGNALS:
	void press(const std::string id, const std::string mode);

private:
	QLabel*			m_id_label;
	QLineEdit*		m_id;
	QComboBox*		m_mode;
	QPushButton*	m_add;

	QVBoxLayout*	m_vlayout;
};

class FacialEditWidget : public QWidget {
	Q_OBJECT
public:
	FacialEditWidget(FacialAnimation *ani, FacialObject* obj, QWidget* parent = nullptr);
	virtual ~FacialEditWidget();

protected:
	void paintEvent(QPaintEvent* event) override;

public slots:
	void addAnimte(const std::string id, const std::string mode);
	void deleteAnimate(AnimateEdit* w, std::string id);

private:
	FacialAnimation* m_ani;
	FacialObject*	 m_face_obj;
	EditVec3		 m_pos;
	EditVec3		 m_scale;
	EditVec3		 m_rotate;

	std::vector<AnimateEdit*>	m_animates;
	AddAnimate*					m_add_animate;

	QVBoxLayout*	 m_vlayout;
};