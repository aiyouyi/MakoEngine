#pragma once
#include <QtCore>
#include "Toolbox/inc.h"
#include "BaseDefine/Vectors.h"

class QSynchronizeConfigEvent : public QEvent
{
public:
	QSynchronizeConfigEvent();
public:
	static int type;//自定义的事件类型
};

class QFurDataEvent : public QEvent
{
public:
	QFurDataEvent();
public:
	static int type;//自定义的事件类型
};

class AddSkeletonLayerEvent : public QEvent
{
public:
	AddSkeletonLayerEvent();
public:
	static int type;//自定义的事件类型
	std::map<std::string, int> bone_name_map;
};

class QPostProcessEvent : public QEvent
{
public:
	QPostProcessEvent();
public:
	static int type;//自定义的事件类型
};