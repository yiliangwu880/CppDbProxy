/*
测试dbproxy_svr是否又内存泄漏。
随机长期运行的方法测试。
*/

#include <string>
#include "libevent_cpp/include/include_all.h"
#include "svr_util/include/su_mgr.h"
#include "svr_util/include/single_progress.h"
#include "svr_util/include/read_cfg.h"
#include "svr_util/include/su_rand.h"
#include "svr_util/include/str_util.h"
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
		WAIT_INIT_TALBE,

		WAIT_RANDOM_END,
	};
	State m_state;
	lc::Timer m_tm; //random handle timer
	void OnCon() {};

	void OnRspInsert2(bool, const Player2 & data) {};
	void OnRspInsert3(bool, const Player3 & data) {};
	void OnQuery3(bool, const Player3 & data) {};
	void OnDel3(bool, const Player3 & data) {};
	void RandomReq();

	void Start()
	{
		m_state = WAIT_CONNECT;
		UNIT_INFO("connect %s %d", CfgMgr::Ins().dbproxy_svr_ip.c_str(), CfgMgr::Ins().dbproxy_svr_port);
		Dbproxy::Ins().Init(CfgMgr::Ins().dbproxy_svr_ip, CfgMgr::Ins().dbproxy_svr_port, OnCon);
		Dbproxy::Ins().RegInsertCb(OnRspInsert2);
		Dbproxy::Ins().RegInsertCb(OnRspInsert3);
		Dbproxy::Ins().RegQueryCb(OnQuery3);
		Dbproxy::Ins().RegDelCb(OnDel3);

		UNIT_INFO("start timer");

		m_state = WAIT_RANDOM_END;
		m_tm.StartTimer(5, RandomReq, true);

	}

	void RandomReq()
	{
		//UNIT_INFO("r");
		uint32 r = Random::RandUint32(0, 13);
		switch (r)
		{
		case 9:
		{
			Player2 d;
			d.id2 = Random::RandUint32(0, 14);
			Dbproxy::Ins().Insert(d);
		}
		break;
		case 13:
		{
			Player4 d;
			d.name = StrNum::NumToStr( Random::RandUint32(0, 14));
			Dbproxy::Ins().Insert(d);
		}
		break;
		case 12:
		{
			Player2 d;
			d.id2 = Random::RandUint32(0, 14);
			d.myblob2= StrNum::NumToStr(Random::RandUint32(0, 14));
			Dbproxy::Ins().Update(d);
		}
		break;
		case 11:
		case 10:
		{
			Player4 d;
			d.name = StrNum::NumToStr(Random::RandUint32(0, 14));
			d.idx = StrNum::NumToStr(Random::RandUint32(0, 14));
			Dbproxy::Ins().Update(d);
		}
		break;
		case 0:
		case 1:
		{
			Player3 d;
			d.id = Random::RandUint32(0, 14);
			Dbproxy::Ins().Insert(d);
		}
		break;
		case 2:
		{
			Player3 d;
			d.id = Random::RandUint32(0, 14);
			Dbproxy::Ins().Query(d);

		}
		break;
		case 3:
		{
			Player4 d;
			d.name = StrNum::NumToStr(Random::RandUint32(0, 14));
			Dbproxy::Ins().Query(d);
		}
		break;
		case 4:
		{
			Player4 d;
			d.name = StrNum::NumToStr(Random::RandUint32(0, 14));
			Dbproxy::Ins().Del(d);
		}
		break;
		case 5:
		{
			Player2 d;
			d.id2 = Random::RandUint32(0, 14);
			Dbproxy::Ins().Del(d);

		}
		break;
		case 6:
		{
			Player2 d;
			d.id2 = Random::RandUint32(0, 14);
			Dbproxy::Ins().Query(d, "id2 > 3 ");

		}
		break;
		case 7:
		{
			Dbproxy::Ins().ExecuteSql("INSERT INTO `TestTable` VALUES ('2', null, '11', null, null, null, null, null)");

		}
		break;
		case 8:
		{
			Dbproxy::Ins().ExecuteSql("delete from TestTable where id=2");
		}
		break;
		default:
			break;
		}
	}


}

UNITTEST(test_leak)
{
	UNIT_ASSERT(CfgMgr::Ins().Init());
	SuMgr::Ins().Init();
	string ss;
	ss = 3;
	Start();

	EventMgr::Ins().Dispatch();
	UNIT_INFO("--------------------test_leak end--------------------");

}