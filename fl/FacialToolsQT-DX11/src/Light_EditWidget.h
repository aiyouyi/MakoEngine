#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <vector>
#include <QLineEdit>
#include <QLabel>
#include <QColorDialog>

#include "EffectKernel2\FacialAnimation.h"

struct EditCluster {
	QLabel*				 m_orientation;
	QLineEdit*			 m_orientation_x;
	QLineEdit*			 m_orientation_y;
	QLineEdit*			 m_orientation_z;

	QLabel*				 m_ambient;
	QLineEdit*			 m_ambient_w;

	QLabel*				 m_diffuse;
	QLineEdit*			 m_diffuse_w;

	QLabel*				 m_specular;
	QLineEdit*			 m_specular_w;

	QVBoxLayout*		 m_vlayout;
};

class LightEditWidget : public QWidget {
	Q_OBJECT
public:
	LightEditWidget(FacialAnimation* ani, QWidget* parent = nullptr);
	virtual ~LightEditWidget();

public slots:
	void editChange(EditCluster* ec, int index);

protected:
	void paintEvent(QPaintEvent* event) override;

private:
	void addClusters();
	

	FacialAnimation*			m_ani;
	std::vector<EditCluster*>	m_cluster;
	QVBoxLayout*				m_vlayout;
};