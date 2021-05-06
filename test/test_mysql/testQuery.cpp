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
	void OnRspInsert2(bool r, const Player2 & data) {};
	void OnRspInsert3(bool r, const Player3 & data) {};
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
	
		Insert();
	}

	int g_query_cnt = 0;
	void Insert()
	{
		UNIT_INFO("Insert");
		m_state = WAIT_INSERT;
		{
			Player3 data;
			data.id = 1;
			data.id2 = data.id;
			data.myblob2 = "中文";
			Dbproxy::Ins().Insert(data);
			data.id = 2;
			data.id2 = data.id;
			Dbproxy::Ins().Insert(data);
			data.id = 3;
			data.id2 = data.id;
			Dbproxy::Ins().Insert(data);
			data.id = 4;
			data.id2 = data.id;
			Dbproxy::Ins().Insert(data);
		}
		{
			Player4 data;
			data.name = "1";
			Dbproxy::Ins().Insert(data);
			data.name = "2";
			Dbproxy::Ins().Insert(data);
			data.name = "3";
			Dbproxy::Ins().Insert(data);	
		}
		//start query
		{
			Player3 data;
			Dbproxy::Ins().Query(data, 100);
			g_query_cnt=1;
		}
	}

	int g_run_cnt = 0;
	lc::Timer tm;
	void OnQuery3(bool ret, const Player3 & data)
	{
		if (g_query_cnt == 1)
		{
			UNIT_ASSERT(ret);

			g_run_cnt++;
			if (g_run_cnt == 4 )
			{
				auto f = []()
				{
					UNIT_ASSERT(g_run_cnt == 4);
					UNIT_INFO("g_query_cnt = 2")
					Player3 data;
					data.id = 1;
					Dbproxy::Ins().Query(data, 100);
					g_query_cnt = 2;
					g_run_cnt = 0;
				};
				L_INFO("start timer g_query_cnt == 1");
				tm.StartTimerSec(2, f);
			}
		}
		else if (2 == g_query_cnt)
		{
			UNIT_ASSERT(g_query_cnt == 2);
			UNIT_ASSERT(ret);
			UNIT_ASSERT(data.id == 1);
			UNIT_ASSERT(data.id2 == 1);

			auto f = []()
			{
				Player3 data;
				Dbproxy::Ins().Query(data, "id > 2", 100);
				g_query_cnt = 3; 
				g_run_cnt = 0;
			};
			L_INFO("start timer g_query_cnt == 2");
			tm.StartTimerSec(2, f);
		}
		else if (3 == g_query_cnt)
		{
			g_run_cnt++;
			UNIT_ASSERT(ret);
			UNIT_ASSERT(data.id == 3 || data.id == 4);
			UNIT_INFO("3 == g_query_cnt data.id=%d", data.id);
		}
	}


	void OnDel3(bool ret, const Player3 & data)
	{
		UNIT_ASSERT(g_run_cnt ==4);
		UNIT_ASSERT(ret);
		UNIT_INFO("OnDel3")
	}

	void OnDiscon()
	{
		UNIT_INFO("OnDiscon");
	}

}

UNITTEST(test_query)
{
	UNIT_ASSERT(CfgMgr::Ins().Init());

	Start();

	EventMgr::Ins().Dispatch();
	UNIT_INFO("--------------------test_mysql end--------------------");

}