#pragma once

#include <QWidget>
#include <QDebug>
#include <QVBoxLayout>
#include <QPushButton>

#include <vector>
#include <map>

#include "common.h"
#include "effectLayerItem.h"

class CEffectPart;

class LayersWidget : public QWidget
{
	Q_OBJECT

public:
	LayersWidget(cc_handle_t sticker_effect, bool AR = true, QWidget* parent = nullptr);
	~LayersWidget();
	
	void changeStrickerAR(cc_handle_t stricker);
	void changeStrickerScene(cc_handle_t stricker);
	void addEffectAR();

Q_SIGNALS:
	void switchWidget(QWidget* w);

protected:
	void mousePressEvent(QMouseEvent *event) override;
	void dragEnterEvent(QDragEnterEvent *event) override;
	void dragMoveEvent(QDragMoveEvent *event) override;
	void dropEvent(QDropEvent *event) override;

private:
	void genEidtWidgetAR();
	void genEidtWidgetScene();
	void AddOneEffectAR(CEffectPart* part);
	void exchangeItem(int src_index, int dst_index);

public slots:
	void chosenWidget(EffectLayerItem* item);
	void testBtn();
	void deleteItemAR(EffectLayerItem* item);

private:
	cc_handle_t			m_StickerEffect;
	QVBoxLayout*		m_layout;
	bool				m_AR;
	QWidget*			m_chosen_widget;

	std::vector<EffectLayerItem*>				m_items_vec;
	std::map<EffectLayerItem*, QWidget*>		m_item_widget_map;
	std::map<EffectLayerItem*, CEffectPart*>	m_item_effectpart_map;
};