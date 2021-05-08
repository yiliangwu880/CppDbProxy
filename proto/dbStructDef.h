/*
db table 嵌套的 struct 定义
*/

#pragma once

#include <string>
#include <vector>
#include "dbTable.h"

namespace db
{

	struct Ride //比如坐骑模块 
	{
		uint32_t id;
		std::vector<uint32_t> ids;
	};
	struct SubRide2 
	{
		uint32_t id;
		std::vector<uint32_t> ids;
	};
	struct Ride2 //比如坐骑模块 
	{
		uint32_t id;
		bool isOk;
		uint32_t id1;
		uint32_t id2;
		SubRide2 sub;
	};
	struct Player2Ride //比如坐骑模块 
	{
		uint32_t id;
		std::string name;
		std::vector<Ride> rides;
	};

//先定义宏参数信息，以后再具体实现
//域的顺序 和 遗漏，都会初始化的时候检查出来
#define DB_ALL_STRUCT_INFO \
	DB_CLASS_NAME(Ride)\
	DB_FIELD(id)\
	DB_FIELD(ids)\
	DB_CLASS_END\
\
	DB_CLASS_NAME(SubRide2)\
	DB_FIELD(id)\
	DB_FIELD(ids)\
	DB_CLASS_END\
\
	DB_CLASS_NAME(Ride2)\
	DB_FIELD(id)\
	DB_FIELD(isOk)\
	DB_FIELD(id1)\
	DB_FIELD(id2)\
	DB_FIELD(sub)\
	DB_CLASS_END\
\
	DB_CLASS_NAME(Player2Ride)\
	DB_FIELD(id)\
	DB_FIELD(name)\
	DB_FIELD(rides)\
	DB_CLASS_END\
\




}