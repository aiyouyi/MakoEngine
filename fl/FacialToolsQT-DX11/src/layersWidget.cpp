#include "layersWidget.h"
#include "EffectCInterface.h"
#include "EffectKernel/CEffectPart.h"
#include "EffectKernel\Sticker\C2DPicTextureEffect.h"
#include "EffectKernel\FaceMakeUp\CFaceMakeUp.h"
#include "C2DPicTextureEffect_EditWidget.h"
#include "Scene2DEffect_EditWidget.h"
#include "CFaceMakeUp_EditWidget.h"
#include "Light_EditWidget.h"
#include "NPR_EditWidget.h"
#include "FacialEditWidget.h"
#include "EffectKernel2\FacialAnimation.h"

#include <QMouseEvent>
#include <QMimeData>
#include <QByteArray>
#include <QDrag>

static QVBoxLayout* getVBoxLayout() {
	QVBoxLayout* l = new QVBoxLayout;
	l->setSpacing(0);
	l->setMargin(0);
	l->setContentsMargins(0, 0, 0, 0);
	return l;
}

LayersWidget::LayersWidget(cc_handle_t sticker_effect, bool AR, QWidget *parent)
	: m_StickerEffect(sticker_effect), m_AR(AR), QWidget(parent) {
	setFixedWidth(150);
	
	if(m_AR)
		genEidtWidgetAR();
	else
		genEidtWidgetScene();
	m_layout = getVBoxLayout();
	for (auto item : m_items_vec) 
		m_layout->addWidget(item, 0);

	m_layout->addStretch(1);

	setAcceptDrops(true);
	setFocusPolicy(Qt::StrongFocus);
	setLayout(m_layout);
}

LayersWidget::~LayersWidget() {}

void LayersWidget::changeStrickerAR(cc_handle_t stricker)
{
	m_AR = true;
	//如果不做这一步，在切换时可能在 EditSwitchWidget::switchWidget(QWidget * w)
	//使用了已经被delete的m_edit_widget
	emit switchWidget(nullptr);

	m_StickerEffect = stricker;
	for (auto item : m_items_vec) {
		m_layout->removeWidget(item);
		delete m_item_widget_map[item];
		delete item;
	}
	m_items_vec.clear();
	m_item_widget_map.clear();

	genEidtWidgetAR();

	delete m_layout;
	m_layout = getVBoxLayout();
	for (int i = 0; i < m_items_vec.size(); i++) {
		m_layout->addWidget(m_items_vec[i]);
	}
	m_layout->addStretch(1);

	setAcceptDrops(true);
	setFocusPolicy(Qt::StrongFocus);

	setLayout(m_layout);
}

void LayersWidget::changeStrickerScene(cc_handle_t stricker)
{
	m_AR = false;
	//如果不做这一步，在切换时可能在 EditSwitchWidget::switchWidget(QWidget * w)
	//使用了已经被delete的m_edit_widget
	emit switchWidget(nullptr);

	m_StickerEffect = stricker;
	for (auto item : m_items_vec) {
		m_layout->removeWidget(item);
		delete m_item_widget_map[item];
		delete item;
	}
	m_items_vec.clear();
	m_item_widget_map.clear();

	genEidtWidgetScene();

	delete m_layout;
	m_layout = getVBoxLayout();
	for (int i = 0; i < m_items_vec.size(); i++) {
		m_layout->addWidget(m_items_vec[i]);
	}
	m_layout->addStretch(1);

	setLayout(m_layout);
}

void LayersWidget::addEffectAR()
{
	int size = 0;
	CEffectPart** parts = reinterpret_cast<CEffectPart**>(ccGetCEffectParts(m_StickerEffect, &size));
	if (parts && 0 != size) {
		CEffectPart* part = parts[size - 1];
		AddOneEffectAR(part);

		delete m_layout;
		m_layout = getVBoxLayout();
		for (int i = 0; i < m_items_vec.size(); i++) {
			m_layout->addWidget(m_items_vec[i]);
		}
		m_layout->addStretch(1);

		setLayout(m_layout);
	}
}

void LayersWidget::mousePressEvent(QMouseEvent * event) {
	EffectLayerItem *child = static_cast<EffectLayerItem*>(childAt(event->pos()));

	if ( m_AR && child && child->inherits("QWidget")) {
		int index = m_layout->indexOf(child);
		if(index >= 0 && index < m_items_vec.size()) {
			QByteArray itemData;
			QDataStream dataStream(&itemData, QIODevice::WriteOnly);
			dataStream << index;

			QMimeData *mimeData = new QMimeData;
			mimeData->setData("application/x-dnditemdata", itemData);

			QDrag *drag = new QDrag(this);
			drag->setMimeData(mimeData);
			drag->setHotSpot(event->pos() - child->pos());

			Qt::DropAction action = drag->exec(Qt::MoveAction);
		}
	}else {
		emit switchWidget(nullptr);
	}
}

void LayersWidget::dragEnterEvent(QDragEnterEvent* event) {
	if (event->mimeData()->hasFormat("application/x-dnditemdata")) {
		if (event->source() == this) {
			event->setDropAction(Qt::MoveAction);
			event->accept();
		} else {
			event->acceptProposedAction();
		}
	} else {
		event->ignore();
	}
}

void LayersWidget::dragMoveEvent(QDragMoveEvent* event) {
	//if (event->mimeData()->hasFormat("application/x-dnditemdata")) {
	//	QByteArray itemData = event->mimeData()->data("application/x-dnditemdata");
	//	QDataStream dataStream(&itemData, QIODevice::ReadOnly);
	//	if (event->source() == this) {
	//		event->setDropAction(Qt::MoveAction);
	//		event->accept();
	//	} else {
	//		event->acceptProposedAction();
	//	}
	//} else {
	//	event->ignore();
	//}
}

void LayersWidget::dropEvent(QDropEvent* event) {
	if (event->mimeData()->hasFormat("application/x-dnditemdata")) {
		QByteArray itemData = event->mimeData()->data("application/x-dnditemdata");
		QDataStream dataStream(&itemData, QIODevice::ReadOnly);

		int old_index;
		dataStream >> old_index;
		int new_index = event->pos().y() / ItemHeight;
		if (new_index >= m_items_vec.size()) {
			new_index = m_items_vec.size() - 1;
		}
		if (old_index != new_index) {
			exchangeItem(old_index, new_index);
		}

		if (event->source() == this) {
			event->setDropAction(Qt::MoveAction);
			event->accept();
		} else {
			event->acceptProposedAction();
		}
	} else {
		event->ignore();
	}
}

void LayersWidget::genEidtWidgetAR()
{
	int size = 0;
	CEffectPart** parts = reinterpret_cast<CEffectPart**>(ccGetCEffectParts(m_StickerEffect, &size));
	if (parts) {
		for (int i = 0; i < size; i++) {
			CEffectPart* part1 = parts[i];
			AddOneEffectAR(part1);
		}
	}
}

void LayersWidget::genEidtWidgetScene() {
	FacialAnimation* ani = reinterpret_cast<FacialAnimation*>(m_StickerEffect);
	vector<BaseLight>& light = ani->getLight();
	if(light.size() > 0) {
		EffectLayerItem* item = new EffectLayerItem(QString::fromLocal8Bit("灯光"), this);
		QWidget* edit = new LightEditWidget(ani, dynamic_cast<QWidget*>(this->parent()));
		edit->setHidden(true);
		m_items_vec.push_back(item);
		m_item_widget_map[item] = edit;
		connect(item, &EffectLayerItem::clicked, this, &LayersWidget::chosenWidget);
	}
	{
		EffectLayerItem* item = new EffectLayerItem(QString::fromLocal8Bit("NPR"), this);
		QWidget* edit = new NPREditWidget(ani, dynamic_cast<QWidget*>(this->parent()));
		edit->setHidden(true);
		m_items_vec.push_back(item);
		m_item_widget_map[item] = edit;
		connect(item, &EffectLayerItem::clicked, this, &LayersWidget::chosenWidget);
	}
	if (0 < ani->getEffect2DRectVec().size()) {
		EffectLayerItem*  item = new EffectLayerItem(QString::fromLocal8Bit("背景动画"), this);
		QWidget* edit = new Scene2DEffectEditWidget(ani, dynamic_cast<QWidget*>(this->parent()));
		edit->setHidden(true);
		m_items_vec.push_back(item);
		m_item_widget_map[item] = edit;
		connect(item, &EffectLayerItem::clicked, this, &LayersWidget::chosenWidget);
	}
	if(ani->getFacialObj()){
		EffectLayerItem*  item = new EffectLayerItem(QString::fromLocal8Bit("主角"), this);
		QWidget* edit = new FacialEditWidget(ani, ani->getFacialObj(), dynamic_cast<QWidget*>(this->parent()));
		edit->setHidden(true);
		m_items_vec.push_back(item);
		m_item_widget_map[item] = edit;
		connect(item, &EffectLayerItem::clicked, this, &LayersWidget::chosenWidget);
	}
}

void LayersWidget::AddOneEffectAR(CEffectPart * part)
{
	EffectLayerItem * item = nullptr;
	QWidget* edit = nullptr;

	C2DPicTextureEffect *pic = dynamic_cast<C2DPicTextureEffect*>(part);
	if (pic) {
		item = new EffectLayerItem(QString::fromLocal8Bit("2D贴图动画"), this, true);
		edit = new C2DPicTextEidtWidget(pic, dynamic_cast<QWidget*>(this->parent()));
	}
	CFaceMakeUp* face = dynamic_cast<CFaceMakeUp*>(part);
	if (face) {
		item = new EffectLayerItem(QString::fromLocal8Bit("美妆"), this, true);
		edit = new CFaceMakeUpEidtWidget(face, dynamic_cast<QWidget*>(this->parent()));
	}
// 	CFaceEffect3D* face3d = dynamic_cast<CFaceEffect3D*>(part);
// 	if (face3d) {
// 		item = new EffectLayerItem(QString::fromLocal8Bit("贴片"), this, true);
// 		edit = new FaceEffect3DEditWidget(face3d, dynamic_cast<QWidget*>(this->parent()));
// 	}

	if (item && edit) {
		edit->setHidden(true);
		m_items_vec.push_back(item);
		m_item_widget_map[item] = edit;
		m_item_effectpart_map[item] = part;
		connect(item, &EffectLayerItem::clicked, this, &LayersWidget::chosenWidget);
		connect(item, &EffectLayerItem::deleted, this, &LayersWidget::deleteItemAR);
	}
}

void LayersWidget::exchangeItem(int src_index, int dst_index) {
	//exchange the part
	bool ex_part = ccExchangePart(m_StickerEffect, src_index, dst_index);
	if(ex_part) {
		//exchange the m_items_vec
		EffectLayerItem* src_item = m_items_vec[src_index];
		EffectLayerItem* dst_item = m_items_vec[dst_index];
		auto src_item_it = std::find(m_items_vec.begin(), m_items_vec.end(), src_item);
		m_items_vec.erase(src_item_it);
		auto dst_item_it = std::find(m_items_vec.begin(), m_items_vec.end(), dst_item);
		if(dst_index > src_index)
			++dst_item_it;
		m_items_vec.insert(dst_item_it, src_item);

		//exchange the EffectLayerItem in layout
		m_layout->removeWidget(src_item);
		m_layout->insertWidget(dst_index, src_item);
		update();
	}
}

void LayersWidget::chosenWidget(EffectLayerItem* item) {
	m_chosen_widget = nullptr;
	if (m_item_widget_map.find(item) != m_item_widget_map.end()) {
		m_chosen_widget = m_item_widget_map[item];
	}
	emit switchWidget(m_chosen_widget);
}

void LayersWidget::testBtn() {
	qDebug() << "test";
	//ccZipAllEffect(m_StickerEffect);
}

void LayersWidget::deleteItemAR(EffectLayerItem* item) {
	if(m_chosen_widget == item)	emit switchWidget(nullptr);
	auto vec_res = std::find(m_items_vec.begin(), m_items_vec.end(), item);
	if (vec_res != m_items_vec.end()) {
		m_items_vec.erase(vec_res);
		delete item;
	}
	auto w_map_res = m_item_widget_map.find(item);
	if (w_map_res != m_item_widget_map.end()) {
		delete w_map_res->second;
		m_item_widget_map.erase(w_map_res);
	}
	auto effect_map_res = m_item_effectpart_map.find(item);
	if (effect_map_res != m_item_effectpart_map.end()) {
		ccDeleteEffect(m_StickerEffect, effect_map_res->second);
		m_item_effectpart_map.erase(effect_map_res);
	}
}
