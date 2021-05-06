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



namespace
{
	struct MyMsg
	{
		const uint16_t id = 2;
		int ret;
		char data[0]; //有值的内容作为条件
	};
	struct MyMsg2
	{
		const uint16_t id = 3;
		char b;
		int ret;
		char data[0]; //有值的内容作为条件
	};

	std::unordered_map<uint16_t, void *> m_cmdId2Cb; //proto 消息ID 2 回调

	template<class MsgType> //Fun类型要求 void fun(const insert_sc &msg), 其中 insert_sc 可变
	void RegProtoParse(void(*fun)(const MsgType &msg))
	{
		//typedef typename std::function<void (const MsgType &msg)>::first_argument_type P1type;
		//typedef typename std::remove_reference<P1type>::type Type1;
		//typedef typename std::remove_const<Type1>::type Type;
		MsgType msg;
		if (m_cmdId2Cb.find(msg.id) != m_cmdId2Cb.end())
		{
			L_ERROR("repeated reg");
			return;
		}
		m_cmdId2Cb[msg.id] = (void *)fun;
	}


	void ParseInsert(const MyMsg &msg)
	{
		UNIT_INFO("MyMsg ret=%d", msg.ret);

	}
	void ParseInsert2(const MyMsg2 &msg)
	{
		UNIT_INFO("MyMsg2 b=%d", msg.b);

	}
}

	


UNITTEST(msgParse)
{
	RegProtoParse(ParseInsert);
	RegProtoParse(ParseInsert2);
	uint32_t len=33;
	{
		MyMsg msg;
		msg.ret = 11;
		using ComFun = void(const char &, decltype(len)); //消息回调函数， 抽象类型。 
		ComFun *fun = (ComFun *)(m_cmdId2Cb[msg.id]);
		const char *pMsg = (char *)&msg;
		(*fun)(*pMsg, len);
	}
	{
		MyMsg2 msg;
		msg.b = 12;
		using ComFun = void(const char &, decltype(len)); //消息回调函数， 抽象类型。 
		ComFun *fun = (ComFun *)(m_cmdId2Cb[msg.id]);
		const char *pMsg = (char *)&msg;
		(*fun)(*pMsg, len);
	}
}