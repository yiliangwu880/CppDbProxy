/*
db table 定义，同时定义 c++对象类型
*/

#pragma once

#include <string>
#include <vector>
#include "dbTable.h"

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
	};
	struct Player3 : public BaseTable
	{
		Player3() : BaseTable("Player3", 3) {}
		uint64_t id =0;
		uint8_t id1  =0;
		uint64_t id2 =0;

		Bytes myblob1;
		int8_t id3   =0;
		std::string myblob2;
	};
	struct Player4 : public BaseTable
	{
		Player4() : BaseTable("Player4", 4) {}
		std::string name;
		std::string idx;
	};
	//抽象
#define DB_ALL_TABLE_INFO \
	DB_CLASS_NAME(Player2)\
	DB_MAIN_KEY(id2)\
	DB_FIELD(myblob)\
	DB_FIELD(myblob2)\
	DB_CLASS_END\
\
	DB_CLASS_NAME(Player3)\
	DB_MAIN_KEY(id)\
	DB_FIELD(id1)\
	DB_INDEX_KEY(id2)\
	DB_FIELD(myblob1)\
	DB_FIELD(id3)\
	DB_FIELD(myblob2)\
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