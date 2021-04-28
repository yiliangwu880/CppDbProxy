/*

*/

#include <string>
#include "libevent_cpp/include/include_all.h"
#include "svr_util/include/su_mgr.h"
#include "svr_util/include/single_progress.h"
#include "svr_util/include/read_cfg.h"
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
	class DbMgr: public db::BaseDbproxy
	{
	public:
		enum State
		{
			WAIT_CONNECT,
			WAIT_DROP_TABLE, //删除上次测试的table
			WAIT_INIT_TALBE,
			WAIT_INSERT,
			WAIT_UPDATE,
			WAIT_GET,
			WAIT_DEL,
			WAIT_SQL_INSERT,
			WAIT_GET_SQL_INSERT,
			WAIT_SQL_DEL,
		};
		State m_state;
		TestTable m_msg;
		SubMsg m_sub;
		TTT3 m_t;
	public:
		DbMgr();

		void Start();
		void StartInitTable();

		virtual void OnCon();
		virtual void OnDiscon();
		virtual void OnRspInitTable(bool is_ok) ;
		virtual void OnRspInsert(const db::RspInsertData &rsp) ;
		virtual void OnRspUpdate(const db::RspUpdateData &rsp) ;
		virtual void OnRspGet(const db::RspGetData &rsp) ;
		virtual void OnRspDel(const db::RspDelData &rsp) ;
		virtual void OnRspSql(bool is_ok);
	};




	DbMgr::DbMgr()
	{
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
		UNIT_INFO("OnRspInitTable");
		UNIT_ASSERT(WAIT_INIT_TALBE == m_state);
		UNIT_ASSERT(is_ok);
		m_state = WAIT_INSERT;
		{
			m_msg.set_id(1);
			m_msg.set_name_32(1);
			m_msg.set_name_str("a");
			m_msg.set_name_enum(T1);
			m_msg.set_name_bool(true);
			m_msg.set_name_bytes("a看到了", 1);
			Insert(m_msg);
		}
		{
			m_sub.set_id(1);
			m_sub.set_name_enum(T1);
			m_sub.set_name_bool(true);

			m_t.set_id(1);
			m_t.set_name("abc");
			m_t.mutable_sub_msg()->CopyFrom(m_sub);
			m_t.set_t23("中文");
			Insert(m_t);
		}
	}


	void DbMgr::OnRspInsert(const db::RspInsertData &rsp)
	{
		UNIT_INFO("OnRspInsert");
		UNIT_ASSERT(WAIT_INSERT == m_state);
		UNIT_ASSERT(rsp.is_ok());
		if (rsp.msg_name() == "TTT3")
		{
			UNIT_ASSERT(rsp.str_key() == m_t.name());
			UNIT_INFO("start updte");
			m_state = WAIT_UPDATE;
			{
				m_msg.set_name_str("c");
				Update(m_msg);
			}
			{
				m_sub.set_id(2);
				m_sub.set_name_enum(T2);
				m_sub.set_name_bool(false);
				m_t.set_t23("b");
				Update(m_t);
			}


		}
		else if (rsp.msg_name() == "TestTable")
		{
			UNIT_ASSERT(rsp.num_key() == m_msg.id());
		}

	}


	void DbMgr::OnRspUpdate(const db::RspUpdateData &rsp)
	{
		UNIT_INFO("OnRspUpdate");
		UNIT_ASSERT(WAIT_UPDATE == m_state);
		UNIT_ASSERT(rsp.is_ok());
		if (rsp.msg_name() == "TTT3")
		{
			UNIT_ASSERT(rsp.str_key() == m_t.name());
			m_state = WAIT_GET;
			UNIT_INFO("start get data");
			Get<TestTable>("id=1");
			Get<TTT3>("name='abc'");
		}
		else if (rsp.msg_name() == "TestTable")
		{
			UNIT_ASSERT(rsp.num_key() == m_msg.id());
		}
	}

	void DbMgr::OnRspGet(const db::RspGetData &rsp)
	{
		UNIT_INFO("OnRspGet");
		if (WAIT_GET == m_state)
		{
			UNIT_ASSERT(rsp.data_size() == 1);
			UNIT_ASSERT(rsp.is_last());
			if (rsp.msg_name() == "TTT3")
			{
				UNIT_ASSERT(rsp.data_size() == 1);
				TTT3 rsp_msg;
				bool r = rsp_msg.ParseFromString(rsp.data(0));
				UNIT_ASSERT(r);
				UNIT_ASSERT(rsp_msg.name() == m_t.name());

				m_state = WAIT_DEL;
				Del<TestTable>(1);
				Del<TTT3>("abc");
			}
			else if (rsp.msg_name() == "TestTable")
			{
				UNIT_ASSERT(rsp.data_size() == 1);
				TestTable rsp_msg;
				bool r = rsp_msg.ParseFromString(rsp.data(0));
				UNIT_ASSERT(r);
				UNIT_ASSERT(rsp_msg.id() == m_msg.id());
				string s = m_msg.SerializeAsString();
				UNIT_ASSERT(s == rsp.data(0));//所有值一样。
			}
		}
		else if (WAIT_GET_SQL_INSERT == m_state)
		{
			UNIT_ASSERT(rsp.data_size() == 1);
			UNIT_ASSERT(rsp.is_last());
			UNIT_ASSERT(rsp.msg_name() == "TestTable");
			TestTable rsp_msg;
			bool r = rsp_msg.ParseFromString(rsp.data(0));
			UNIT_ASSERT(r);
			UNIT_ASSERT(rsp_msg.id() == 2);
			UNIT_ASSERT(rsp_msg.name_64() == 11);
			UNIT_INFO("get sql insert del end");
			m_state = WAIT_SQL_DEL;
			ExecuteSql("delete from TestTable where id=2");
		}
		else
		{
			UNIT_ASSERT(false);
		}

	}

	void DbMgr::OnRspDel(const db::RspDelData &rsp)
	{
		UNIT_INFO("OnRspDel");
		UNIT_ASSERT(WAIT_DEL == m_state);
		UNIT_ASSERT(rsp.del_num() == 1);
		if (rsp.msg_name() == "TestTable")
		{
			UNIT_ASSERT(rsp.num_key() == 1);
			UNIT_ASSERT(rsp.str_key() == "");
		}
		else if (rsp.msg_name() == "TTT3")
		{
			UNIT_INFO("del end");
			UNIT_ASSERT(rsp.num_key() == 0);
			UNIT_ASSERT(rsp.str_key() == "abc");
			m_state = WAIT_SQL_INSERT;
			ExecuteSql("INSERT INTO `TestTable` VALUES ('2', null, '11', null, null, null, null, null)");
		}
	}

	void DbMgr::OnRspSql(bool is_ok)
	{
		static uint32 drop_cnt = 0;
		if(WAIT_SQL_INSERT == m_state)
		{
			UNIT_ASSERT(is_ok);
			m_state = WAIT_GET_SQL_INSERT;
			UNIT_INFO("start get sql insert data");
			Get<TestTable>("id=2");
		}
		else if (WAIT_SQL_DEL == m_state)
		{
			UNIT_ASSERT(is_ok);
			UNIT_INFO("sql del end");
			EventMgr::Obj().StopDispatch();
		}
		else if (WAIT_DROP_TABLE == m_state)
		{
			UNIT_INFO("rsp sql drop table");
			drop_cnt++;
			if (drop_cnt == 2)
			{
				StartInitTable();
			}
		}
	}

	class TC : public lc::ClientCon
	{
	public:
		//每次接收都是完整消息包
		virtual void OnRecv(const MsgPack &msg){};
		virtual void OnConnected() {
			UNIT_INFO("OnConnected");
		};
		virtual void OnError(short events) {};
		//被动删除对象回调，对方断开，或者网络错误
		//被调用的时候， fd, bufferevent 资源已经释放
		//删除本对象， 不会触发OnDisconnected了
		virtual void OnDisconnected() {
			UNIT_INFO("OnDisconnected");
		};
	};
}

UNITTEST(test_mysql)
{
	UNIT_ASSERT(CfgMgr::Obj().Init());
	EventMgr::Obj().Init();

	DbMgr db;
	db.Start();

	EventMgr::Obj().Dispatch();
	UNIT_INFO("--------------------test_mysql end--------------------");

}