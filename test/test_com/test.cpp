/*
brief: use example and test code
*/
#include "../com/unit_test.h"
#include <functional>
#include <limits>
#include <queue>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include "function_traits.h"
#include "db_driver/include/db_driver.h"

using namespace std;
using namespace lc;
using namespace su;
using namespace db;
using namespace proto;


void QueryCbPlayer2(bool isOk, const Player2 &t)
{

}
void QueryCbPlayer3(bool isOk, const Player3 &t)
{

}
void InsertCbPlayer3(bool isOk, const Player3 &t)
{

}
UNITTEST(t334)
{
	BaseDbproxy::Ins().RegQueryCb(QueryCbPlayer2);
	BaseDbproxy::Ins().RegQueryCb(QueryCbPlayer3);
	BaseDbproxy::Ins().RegInsertCb(InsertCbPlayer3);
	TableCfg::Ins();
}


#pragma pack(1)
namespace proto {
	struct DbData  //任意类型
	{
		int i;
	};

	struct Insert
	{
		const uint16_t id = 1;
		int i1, i2;
		char dbData[0];
	};
	struct Query
	{
		const uint16_t id = 2;
		char dbData[0];
	};
	//后跟struct table
	//包含uint16_t tableId; //标识 table,利用宏先后顺序定义
}
#pragma pack()

using namespace proto;
/*

// reg id : fun, size
//parse
void parse(Cmd &cmd, size_t len)
{
	//cmd.id to fun
	//check len size
	info = map[cmd.id];
	info.fun(cmd);
}

template<class Insert>
struct FunTrait
{
	using Fun = void ParseInsert(Insert &msg);
};
template<class Insert, void *funAddr>
void fun<Insert>(Cmd &cmd)
{
	auto fun = (FunTrait<Insert>::Fun)funAddr;
	fun((Insert&)cmd);
}



map[3] = { fun<Insert>,ParseInsert }
//用模板获取函数类型，简化为
map[3] = ParseInsert


input ParseInsert
ouput void fun<Insert>(Cmd &cmd)

*/

using namespace std;

namespace
{
	template<class DbData>
	void send(const Insert &msg, const DbData &db)
	{
		size_t t = sizeof(msg);
		t = sizeof(db);
		if (t)
		{
		}
		//::send(len)//len = cmd size + db size
		//::send(cmd);
		//::send(db);
	}

	void ParseInsert(Insert *msg)
	{
		//cmd.id to fun
	}
	//void ParseQueryResultFun(QueryResult &msg)
	//{
	//	DbTable db = GetDb(msg);
	//}

	using ComFun = void(void *msg);
	using TParseInsert = void(Insert *msg);
}

std::map<uint16_t, void *> g_cmdId2Cb;

template<class Insert, class TParseInsert>
void Reg( TParseInsert fun)
{
	Insert t;
	g_cmdId2Cb[t.id] = (void *)fun;
}

void Rev(uint16_t id, uint8_t *msg)
{
	ComFun *fun = (ComFun *)(g_cmdId2Cb[id]);
	(*fun)(msg);
}
void ParseInsertt1(Insert *msg)
{
	UNIT_ASSERT(msg->i1 == 1);
	UNIT_ASSERT(msg->i2 == 2);
	UNIT_INFO("ParseInsertt1");
	UNIT_INFO("%p ",msg);
}
UNITTEST(ttt)
{
	Reg<Insert>(ParseInsertt1);

	Insert msg;
	UNIT_INFO("%p ", &msg);
	msg.i1 = 1;
	msg.i2 = 2;
	Rev(1, (uint8_t *)&msg);


	DbData t;
	send(msg, t);
}

UNITTEST(ttfun)
{
	// 获取函数第一个入参类型
	//using P1Type = function_traits<decltype(ParseInsert)>::args<0>::type;
	//UNIT_ASSERT(typeid(P1Type) == typeid(Insert));
	//Insert msg;
	//function_traits<decltype(ParseInsert)>::args<0>::type t=&msg;
	//using P1Type = function_traits<decltype(ParseInsert)>::args<0>::type;
	//UNIT_INFO("%d", t->id);
	//UNIT_ASSERT(typeid(P1Type) == typeid(Insert&));
	//UNIT_ASSERT(typeid(P1Type) == typeid(Insert));

}

