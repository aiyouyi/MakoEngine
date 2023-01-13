#include "QUIEvent.h"

int QSynchronizeConfigEvent::type = QEvent::registerEventType();
QSynchronizeConfigEvent::QSynchronizeConfigEvent() :QEvent(QEvent::Type(type))
{

}

int QFurDataEvent::type = QEvent::registerEventType();
QFurDataEvent::QFurDataEvent() :QEvent(QEvent::Type(type))
{

}

int AddSkeletonLayerEvent::type = QEvent::registerEventType();
AddSkeletonLayerEvent::AddSkeletonLayerEvent() :QEvent(QEvent::Type(type))
{

}

int QPostProcessEvent::type = QEvent::registerEventType();
QPostProcessEvent::QPostProcessEvent() :QEvent(QEvent::Type(type))
{

}
