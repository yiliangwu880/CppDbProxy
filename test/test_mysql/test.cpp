/*
大体测试，需要自己手动把db表清除再运行才通过
*/

#include <string>
#include "libevent_cpp/include/include_all.h"
#include "svr_util/include/su_mgr.h"
#include "svr_util/include/single_progress.h"
#include "svr_util/include/read_cfg.h"
#include "unit_test.h"
#include "../db_driver/include/db_driver.h"
#include "../com/cfg.h"

using namespace std;
using namespace su;
using namespace db;
using namespace lc;


namespace
{
	enum State
	{
		WAIT_CONNECT,
		WAIT_DROP_TABLE, //删除上次测试的table
		WAIT_INSERT,
		WAIT_UPDATE,
		WAIT_GET,
		WAIT_DEL,
		WAIT_SQL_INSERT,
		WAIT_GET_SQL_INSERT,
		WAIT_SQL_DEL,
	};
	State m_state;

	 void OnCon();
	 void Start();
	 void StartInitTable();

	 void OnCon();
	 void OnDiscon();

	void Insert();
	void OnRspInsert2(bool, const Player2 & data);
	void OnRspInsert3(bool, const Player3 & data);
	void OnQuery3(bool, const Player3 & data);
	void OnDel3(bool, const Player3 & data);

	void Start()
	{
		m_state = WAIT_CONNECT;
		UNIT_INFO("connect %s %d", CfgMgr::Ins().dbproxy_svr_ip.c_str(), CfgMgr::Ins().dbproxy_svr_port);
		Dbproxy::Ins().Init(CfgMgr::Ins().dbproxy_svr_ip, CfgMgr::Ins().dbproxy_svr_port, OnCon);
		Dbproxy::Ins().RegInsertCb(OnRspInsert2);
		Dbproxy::Ins().RegInsertCb(OnRspInsert3);
		Dbproxy::Ins().RegQueryCb(OnQuery3);
		Dbproxy::Ins().RegDelCb(OnDel3);

	}

	void OnCon()
	{
		UNIT_INFO("onCon");
		UNIT_INFO("Insert");
		m_state = WAIT_INSERT;
		{
			Player2 data;
			data.id2 = 1;
			data.myblob = "a";
			data.myblob2 = "a看到了";
			Dbproxy::Ins().Insert(data);
		}
		{
			Player3 data;
			data.id = 1;
			data.id2 = 1;
			data.myblob2 = "中文";
			Dbproxy::Ins().Insert(data);
		}
		{
			Player4 data;
			data.name = "a";
			data.idx = "中文";
			Dbproxy::Ins().Insert(data);
		}
	}




	void OnRspInsert2(bool is_ok, const Player2 & data)
	{
		UNIT_ASSERT(WAIT_INSERT == m_state);
		UNIT_ASSERT(is_ok);
	}
	void OnRspInsert3(bool is_ok, const Player3 & data)
	{
		UNIT_INFO("OnRspInsert3");
		UNIT_ASSERT(WAIT_INSERT == m_state);
		UNIT_ASSERT(is_ok);
		UNIT_ASSERT(data.id2 == 1);
		UNIT_ASSERT(data.myblob1.empty());
		UNIT_ASSERT(data.myblob2 == "中文");

		UNIT_INFO("start updte");
		m_state = WAIT_UPDATE;

		{
			Player3 data;
			data.id = 1;
			data.id2 = 1;
			data.myblob2 = "中文2";
			Dbproxy::Ins().Update(data);
		}
		{
			Player3 data;
			data.id = 1;
			Dbproxy::Ins().Query(data);
		}
	}

	void OnQuery3(bool ret, const Player3 & data)
	{
		UNIT_ASSERT(ret);

		UNIT_ASSERT(data.id2 == 1);
		UNIT_ASSERT(data.myblob1.empty());
		UNIT_ASSERT(data.myblob2 == "中文2");
		{
			Player3 data;
			data.id = 1;
			data.myblob2 = "ab";
			Dbproxy::Ins().Del(data);
		}
	}



	void OnDel3(bool ret, const Player3 & data)
	{
		UNIT_ASSERT(data.id==1);
		UNIT_ASSERT(ret);
		UNIT_INFO("OnDel3")
	}

	void OnDiscon()
	{
		UNIT_INFO("OnDiscon");
	}

}

UNITTEST(test_mysql)
{
	UNIT_ASSERT(CfgMgr::Ins().Init());

	//Start();

	EventMgr::Ins().Dispatch();
	UNIT_INFO("--------------------test_mysql end--------------------");

}