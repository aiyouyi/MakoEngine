#pragma once
#include <QWidget>
#include <QLabel>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QPushButton>

constexpr int ItemHeight = 30;

class EffectLayerItem : public QWidget
{
	Q_OBJECT

public:
	EffectLayerItem(QString name, QWidget *parent = nullptr,  bool AR = false);
	~EffectLayerItem();

Q_SIGNALS:
	void clicked(EffectLayerItem* item);
	void deleted(EffectLayerItem* item);

protected:
	virtual void mousePressEvent(QMouseEvent *event) override;

private:
	QLabel*		 m_name;
	QCheckBox*	 m_box;
	QHBoxLayout* m_layout;
	QPushButton* m_delete_btn;
	bool m_AR = false;
};