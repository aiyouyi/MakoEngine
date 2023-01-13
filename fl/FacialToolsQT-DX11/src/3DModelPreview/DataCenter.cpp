#include "DataCenter.h"

DataCenter::DataCenter()
{

}

DataCenter::~DataCenter()
{

}

DataCenter& DataCenter::getInstance()
{
	static DataCenter Center;
	return Center;
}
