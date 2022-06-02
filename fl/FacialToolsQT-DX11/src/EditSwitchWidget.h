#pragma once

#include <QWidget>
#include <QVBoxLayout>

class EditSwitchWidget : public QWidget {
	Q_OBJECT
public:
	EditSwitchWidget(QWidget* parent = nullptr);
	virtual ~EditSwitchWidget();

	void switchWidget(QWidget* w);

protected:
	void paintEvent(QPaintEvent* event) override;

private:
	QWidget*		m_edit_widget;
	QVBoxLayout*	m_layout;
};