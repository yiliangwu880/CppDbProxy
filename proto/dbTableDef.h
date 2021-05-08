/*
db table 定义
unordered_map 比较少用，新曾的需要定义对应类型的 packMap，UnpackMap 函数

*/

#pragma once

#include <string>
#include <vector>
#include "dbTable.h"
#include "dbStructDef.h"

namespace db
{
	//测试用,模板参考用
	struct PlayerTest : public BaseTable
	{
		PlayerTest() : BaseTable("PlayerTest", 1) {}
		uint64_t id;
		std::string myblob;
		std::string name;
	};

	struct Player2 : public BaseTable
	{
		Player2() : BaseTable("Player2", 2) {}
		uint64_t id2;
		std::string myblob;
		std::string myblob2;
		Player2Ride ride;
		std::vector<uint8_t> vec8;
		std::unordered_map<uint32_t, uint64_t> m1;
		std::unordered_map<uint32_t, Ride> m2;
	};

	struct Player3 : public BaseTable
	{
		Player3() : BaseTable("Player3", 3) {}
		uint64_t id =0;
		uint32_t id1  =0;
		uint64_t id2 =0;

		Bytes myblob1;
		uint32_t id3   =0;
		std::string myblob2;
		Ride ride;
		Ride2 ride2;
	};
	struct Player4 : public BaseTable
	{
		Player4() : BaseTable("Player4", 4) {}
		std::string name;
		std::string idx;
	};

//先定义宏参数信息，以后再具体实现
//域的顺序 和 遗漏，都会初始化的时候检查出来
#define DB_ALL_TABLE_INFO \
	DB_CLASS_NAME(Player2)\
	DB_MAIN_KEY(id2)\
	DB_FIELD(myblob)\
	DB_FIELD(myblob2)\
	DB_FIELD(ride)\
	DB_FIELD(vec8)\
	DB_FIELD(m1)\
	DB_FIELD(m2)\
	DB_CLASS_END\
\
	DB_CLASS_NAME(Player3)\
	DB_MAIN_KEY(id)\
	DB_FIELD(id1)\
	DB_INDEX_KEY(id2)\
	DB_FIELD(myblob1)\
	DB_FIELD(id3)\
	DB_FIELD(myblob2)\
	DB_FIELD(ride)\
	DB_FIELD(ride2)\
	DB_CLASS_END\
\
	DB_CLASS_NAME(Player4)\
	DB_MAIN_KEY(name)\
	DB_INDEX_KEY(idx)\
	DB_CLASS_END\


//其中具体实现例子，这里为空实现
#define DB_CLASS_NAME(className)	 
#define DB_MAIN_KEY(fieldName)	 
#define DB_INDEX_KEY(fieldName)	
#define DB_FIELD(fieldName)	 
#define DB_CLASS_END		

	DB_ALL_TABLE_INFO

#undef  DB_CLASS_NAME
#undef  DB_MAIN_KEY
#undef  DB_INDEX_KEY
#undef  DB_FIELD
#undef  DB_CLASS_END



}