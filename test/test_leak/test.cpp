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
#include "unit_test.h"
#include "../db_driver/include/db_driver.h"
#include "../com/cfg.h"
#include "proto/test_data.pb.h"

using namespace std;
using namespace su;
using namespace db;
using namespace lc;


namespace
{
	class DbMgr : public db::BaseDbproxy
	{
	public:
		enum State
		{
			WAIT_CONNECT,
			WAIT_DROP_TABLE, //删除上次测试的table
			WAIT_INIT_TALBE,

			WAIT_RANDOM_END,
		};
		State m_state;
		TestTable m_msg;
		SubMsg m_sub;
		TTT3 m_t;
		TTT2 m_t2;
		TTT1 m_t1;
		lc::Timer m_tm; //random handle timer
	public:
		DbMgr();

		void Start();
		void StartInitTable();
		void RandomReq();

		virtual void OnCon();
		virtual void OnDiscon();
		virtual void OnRspInitTable(bool is_ok);
		virtual void OnRspInsert(const db::RspInsertData &rsp);
		virtual void OnRspUpdate(const db::RspUpdateData &rsp);
		virtual void OnRspGet(const db::RspGetData &rsp);
		virtual void OnRspDel(const db::RspDelData &rsp);
		virtual void OnRspSql(bool is_ok);
	};




	DbMgr::DbMgr()
	{

		m_msg.set_id(1);
		m_msg.set_name_32(1);
		m_msg.set_name_str("a");
		m_msg.set_name_enum(T1);
		m_msg.set_name_bool(true);
		m_msg.set_name_bytes("a", 1);

		m_sub.set_id(1);
		m_sub.set_name_enum(T1);
		m_sub.set_name_bool(true);

		m_t.set_id(1);
		m_t.set_name("abc");
		m_t.mutable_sub_msg()->CopyFrom(m_sub);
		m_t.set_t23("a");

		m_t1.set_id(2);
		m_t1.set_t23("ab");

		m_t2.set_id("ab");
		m_t2.set_t(3);
	}


	void DbMgr::Start()
	{
		m_state = WAIT_CONNECT;
		UNIT_INFO("connect %s %d", CfgMgr::Obj().dbproxy_svr_ip.c_str(), CfgMgr::Obj().dbproxy_svr_port);
		Connect(CfgMgr::Obj().dbproxy_svr_ip, CfgMgr::Obj().dbproxy_svr_port);
	}


	void DbMgr::StartInitTable()
	{
		m_state = WAIT_INIT_TALBE;
		ReqInitTable req;
		//TestTable a;
		req.add_msg_name("TestTable");
		req.add_msg_name("TTT1");
		req.add_msg_name("TTT2");
		req.add_msg_name("TTT3");
		InitTable(req);
	}


	void DbMgr::OnCon()
	{
		UNIT_ASSERT(m_state == WAIT_CONNECT);
		m_state = WAIT_DROP_TABLE;
		ExecuteSql("DROP TABLE TTT3");
		ExecuteSql("DROP TABLE TestTable");
	}


	void DbMgr::OnDiscon()
	{
		UNIT_INFO("OnDiscon");
	}


	void DbMgr::OnRspInitTable(bool is_ok)
	{
		UNIT_ASSERT(WAIT_INIT_TALBE == m_state);

		m_state = WAIT_RANDOM_END;
		UNIT_INFO("start timer");
		m_tm.StartTimer(5, std::bind(&DbMgr::RandomReq, this), true);
	}

	void DbMgr::RandomReq()
	{
		//UNIT_INFO("r");
		uint32 r = Random::RandUint32(0, 14);
		switch (r)
		{
		case 14:
		{
			Insert(m_t1);
		}
		break;
		case 13:
		{
			Insert(m_t2);
		}
		break;
		case 12:
		{
			Update(m_t1);
		}
		break;
		case 11:
		{
			Update(m_t2);
		}
		break;
		case 10:
		{
			Update(m_t);
		}
		break;
		case 0:
		{
			Insert(m_msg);
		}
		break;
		case 1:
		{
			Insert(m_t);
		}
		break;
		case 2:
		{
			Get<TestTable>("id=1");

		}
		break;
		case 3:
		{
			Get<TTT3>("name='abc'");

		}
		break;
		case 4:
		{
			Del<TestTable>(1);

		}
		break;
		case 5:
		{
			Del<TTT3>("abc");

		}
		break;
		case 6:
		{
			Get<TestTable>("id=2");

		}
		break;
		case 7:
		{
			ExecuteSql("INSERT INTO `TestTable` VALUES ('2', null, '11', null, null, null, null, null)");

		}
		break;
		case 8:
		{

			ExecuteSql("delete from TestTable where id=2");
		}
		break;
		case 9:
		{
			Update(m_msg);
		}
		break;
		default:
			UNIT_ASSERT(false);
			break;
		}
	}


	void DbMgr::OnRspInsert(const db::RspInsertData &rsp)
	{
	}


	void DbMgr::OnRspUpdate(const db::RspUpdateData &rsp)
	{

	}

	void DbMgr::OnRspGet(const db::RspGetData &rsp)
	{

	}

	void DbMgr::OnRspDel(const db::RspDelData &rsp)
	{

	}

	void DbMgr::OnRspSql(bool is_ok)
	{
		if (WAIT_DROP_TABLE == m_state)
		{
			StartInitTable();
		}
	}
}

UNITTEST(test_leak)
{
	UNIT_ASSERT(CfgMgr::Obj().Init());
	SuMgr::Obj().Init();
	EventMgr::Obj().Init();

	DbMgr db;
	db.Start();

	EventMgr::Obj().Dispatch();
	UNIT_INFO("--------------------test_leak end--------------------");

}