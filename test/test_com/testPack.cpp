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
#include "external/svr_util/include/easy_code.h"
#include "external/svr_util/include/str_util.h"

using namespace std;
using namespace lc;
using namespace su;
using namespace db;
using namespace proto;


UNITTEST(testPack)
{
	{ //只用main key
		Player3 player;
		player.id = 5;
		char ar[1024];
		size_t len = ArrayLen(ar);
		bool r = TableCfg::Ins().Pack(player, ar, len);
		UNIT_ASSERT(r);
		string out(ar, len);
		unique_ptr<BaseTable> p = TableCfg::Ins().Unpack(ar, len);
		UNIT_ASSERT(p != nullptr);
		Player3 &unpackPlayer = *(Player3 *)(p.get());
		UNIT_ASSERT(unpackPlayer.id == player.id);
		UNIT_ASSERT(unpackPlayer.id == 5);
		UNIT_ASSERT(unpackPlayer.id1 == 0);
		UNIT_ASSERT(unpackPlayer.id2 == 0);
		UNIT_ASSERT(unpackPlayer.id3 == 0);
		UNIT_ASSERT(unpackPlayer.myblob1.empty());
		UNIT_ASSERT(unpackPlayer.myblob2.empty());
	}
	{//只用部分字段
		Player3 player;
		player.id = 0xff00ff00ff00ee;
		player.id1 = 3;
		player.id2 = 5;

		char ar[1024];
		size_t len = ArrayLen(ar);
		bool r = TableCfg::Ins().Pack(player, ar, len);
		UNIT_ASSERT(r);
		//UNIT_INFO("len = %d", len);
		string out(ar, len);
		//UNIT_INFO("ar = %s", StrUtil::BinaryToHex(out).c_str());
		
		unique_ptr<BaseTable> p = TableCfg::Ins().Unpack(ar, len);
		UNIT_ASSERT(p != nullptr);
		Player3 &unpackPlayer = *(Player3 *)(p.get());
		UNIT_ASSERT(unpackPlayer.id == player.id);
		UNIT_ASSERT(unpackPlayer.id1 == player.id1);
		UNIT_ASSERT(unpackPlayer.id2 == player.id2);
		UNIT_ASSERT(unpackPlayer.id3 == 0);
		UNIT_ASSERT(unpackPlayer.myblob1.empty());
		UNIT_ASSERT(unpackPlayer.myblob2.empty());
	}

	{//部分字段是 bytes
		Player3 player;
		player.id = 55;
		player.myblob1.append(1, 0);
		player.myblob1.append(1, 11);
		player.myblob1.append(1, 22);

		/////////////////////////
		char ar[1024];
		size_t len = ArrayLen(ar);
		bool r = TableCfg::Ins().Pack(player, ar, len);
		UNIT_ASSERT(r);
		string out(ar, len);
		unique_ptr<BaseTable> p = TableCfg::Ins().Unpack(ar, len);
		UNIT_ASSERT(p != nullptr);
		/////////////////////////

		Player3 &unpackPlayer = *(Player3 *)(p.get());
		UNIT_ASSERT(unpackPlayer.id == player.id);
		UNIT_ASSERT(unpackPlayer.id == 55);
		UNIT_ASSERT(unpackPlayer.id2 == 0);
		UNIT_ASSERT(unpackPlayer.id3 == 0);
		UNIT_ASSERT(!unpackPlayer.myblob1.empty());
		UNIT_ASSERT(unpackPlayer.myblob2.empty());

		UNIT_ASSERT(3 == unpackPlayer.myblob1.size());
		UNIT_ASSERT(0 == unpackPlayer.myblob1[0]);
		UNIT_ASSERT(11 == unpackPlayer.myblob1[1]);
		UNIT_ASSERT(22 == unpackPlayer.myblob1[2]);
	}
	{//部分字段是 string
		Player3 player;
		player.id = 55;
		player.myblob2 = "abc";

		/////////////////////////
		char ar[1024];
		size_t len = ArrayLen(ar);
		bool r = TableCfg::Ins().Pack(player, ar, len);
		UNIT_ASSERT(r);
		string out(ar, len);
		unique_ptr<BaseTable> p = TableCfg::Ins().Unpack(ar, len);
		UNIT_ASSERT(p != nullptr);
		/////////////////////////

		Player3 &unpackPlayer = *(Player3 *)(p.get());
		UNIT_ASSERT(unpackPlayer.id == player.id);
		UNIT_ASSERT(unpackPlayer.id == 55);
		UNIT_ASSERT(unpackPlayer.id2 == 0);
		UNIT_ASSERT(unpackPlayer.id3 == 0);
		UNIT_ASSERT(unpackPlayer.myblob1.empty());
		UNIT_ASSERT(!unpackPlayer.myblob2.empty());

		UNIT_ASSERT(3 == unpackPlayer.myblob2.size());
		UNIT_ASSERT("abc" == unpackPlayer.myblob2);
	} 
	{//填满
		Player3 player;
		player.id = 1;
		player.id1 = 1;
		player.id2 = 2;
		player.id3 = 3;
		player.myblob1.append("1234");
		player.myblob2 = "abc";

		/////////////////////////
		char ar[1024];
		size_t len = ArrayLen(ar);
		bool r = TableCfg::Ins().Pack(player, ar, len);
		UNIT_ASSERT(r);
		string out(ar, len);
		unique_ptr<BaseTable> p = TableCfg::Ins().Unpack(ar, len);
		UNIT_ASSERT(p != nullptr);
		/////////////////////////

		Player3 &unpackPlayer = *(Player3 *)(p.get());
		UNIT_ASSERT(unpackPlayer.id == player.id);
		UNIT_ASSERT(unpackPlayer.id == 1);
		UNIT_ASSERT(unpackPlayer.id1 == 1);
		UNIT_ASSERT(unpackPlayer.id2 == 2);
		UNIT_ASSERT(unpackPlayer.id3 == 3);
		UNIT_ASSERT(!unpackPlayer.myblob1.empty());
		UNIT_ASSERT(!unpackPlayer.myblob2.empty());

		UNIT_ASSERT(unpackPlayer.myblob1 == "1234");
		UNIT_ASSERT("abc" == unpackPlayer.myblob2);
	}
	{//填满
		Player4 player;
		player.name = "abc";
		player.idx = "123";

		/////////////////////////
		char ar[1024];
		size_t len = ArrayLen(ar);
		bool r = TableCfg::Ins().Pack(player, ar, len);
		UNIT_ASSERT(r);
		string out(ar, len);
		unique_ptr<BaseTable> p = TableCfg::Ins().Unpack(ar, len);
		UNIT_ASSERT(p != nullptr);
		/////////////////////////

		Player4 &unpackPlayer = *(Player4 *)(p.get());
		UNIT_ASSERT(unpackPlayer.name == string("abc"));
		UNIT_ASSERT(unpackPlayer.idx == "123");
	
	}
}

