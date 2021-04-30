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
		char data[0]; //��ֵ��������Ϊ����
	};
	struct MyMsg2
	{
		const uint16_t id = 3;
		char b;
		int ret;
		char data[0]; //��ֵ��������Ϊ����
	};

	std::unordered_map<uint16_t, void *> m_cmdId2Cb; //proto ��ϢID 2 �ص�

	template<class Fun> //Fun����Ҫ�� void (const insert_sc &msg, uint32_t len);, ���� insert_sc�ɱ�
	void RegProtoParse(Fun fun)
	{
		typedef typename std::function<Fun>::first_argument_type P1type;
		typedef typename std::remove_reference<P1type>::type Type1;
		typedef typename std::remove_const<Type1>::type Type;
		Type msg;
		if (m_cmdId2Cb.find(msg.id) != m_cmdId2Cb.end())
		{
			L_ERROR("repeated reg");
			return;
		}
		m_cmdId2Cb[msg.id] = (void *)fun;
	}


	void ParseInsert(const MyMsg &msg, uint32_t len)
	{
		UNIT_INFO("MyMsg ret=%d", msg.ret);

	}
	void ParseInsert2(const MyMsg2 &msg, uint32_t len)
	{
		UNIT_INFO("MyMsg2 b=%d", msg.b);

	}
}

	


UNITTEST(msgParse)
{
	RegProtoParse<decltype(ParseInsert)>(ParseInsert);
	RegProtoParse<decltype(ParseInsert2)>(ParseInsert2);
	uint32_t len=33;
	{
		MyMsg msg;
		msg.ret = 11;
		using ComFun = void(const char &, decltype(len)); //��Ϣ�ص������� �������͡� 
		ComFun *fun = (ComFun *)(m_cmdId2Cb[msg.id]);
		const char *pMsg = (char *)&msg;
		(*fun)(*pMsg, len);
	}
	{
		MyMsg2 msg;
		msg.b = 12;
		using ComFun = void(const char &, decltype(len)); //��Ϣ�ص������� �������͡� 
		ComFun *fun = (ComFun *)(m_cmdId2Cb[msg.id]);
		const char *pMsg = (char *)&msg;
		(*fun)(*pMsg, len);
	}
}