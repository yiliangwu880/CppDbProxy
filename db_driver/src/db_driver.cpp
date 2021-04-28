
#include "db_driver.h"
#include "log_def.h"
#include "connect.h"

using namespace std;
using namespace lc;
using namespace su;
using namespace db;
using namespace proto;

/*
db::DbDriverMgr::DbDriverMgr()
	:m_con(*(new DbClientCon()))
{

}

DbDriverMgr::~DbDriverMgr()
{
	delete &m_con;
}

BaseDbproxy::BaseDbproxy()
{
	
}

bool db::BaseDbproxy::Connect(const std::string &ip, uint16 port)
{
	event_base *p = EventMgr::Obj().GetEventBase();
	L_COND_F(p, "must call EventMgr::Obj().Init before this function");
	return m_con.ConnectInit(ip.c_str(), port);
}

bool db::BaseDbproxy::InitTable(const db::ReqInitTable &req)
{
	return m_con.Send(CMD_INIT_TABLE, req);
}



bool db::BaseDbproxy::Update(const google::protobuf::Message &msg)
{
	ReqUpdateData req;
	req.set_msg_name(msg.GetTypeName());
	req.set_data(msg.SerializeAsString());
	return m_con.Send(CMD_UPDATE, req);
}

bool db::BaseDbproxy::Get(const std::string &msg_name, const std::string &cond, uint32 limit_num)
{
	ReqGetData req;
	req.set_msg_name(msg_name);
	req.set_cond(cond);
	req.set_limit_num(limit_num);
	return m_con.Send(CMD_GET, req);
}

bool db::BaseDbproxy::Del(const std::string &msg_name, ::uint64 num_key, const std::string &str_key)
{
	ReqDelData req;
	req.set_msg_name(msg_name);
	req.set_num_key(num_key);
	req.set_str_key(str_key);
	return m_con.Send(CMD_DEL, req);
}

bool db::BaseDbproxy::ExecuteSql(const std::string &sql_str)
{
	ReqSql req;
	req.set_exe_str(sql_str);
	return m_con.Send(CMD_SQL, req);
}
*/

//bool db::DbDriverMgr::Connect(const std::string &ip, uint16 port)
//{
//
//}
namespace
{
	uint32_t CreateId()
	{
		static uint32_t id_seed = 0;
		id_seed++;
		if (id_seed == 0)
		{
			id_seed++;
		}
		return id_seed;
	}
}
db::BaseDbproxy::BaseDbproxy()
	:m_con(*( new DbClientCon()))
{
}


void db::BaseDbproxy::Init(const std::string &ip, uint16_t port, ConCb cb)
{
	m_con.ConnectInit(ip.c_str(), port);
	m_conCb = cb;
}

bool db::BaseDbproxy::Insert(const db::BaseTable &dbObj)
{
	using ProtoType = insert_cs;
	MsgPack msg;
	ProtoType *p = new (msg.data)ProtoType;

	//dbObj可用长度
	size_t len = ArrayLen(msg.data) - sizeof(ProtoType);
	if (!TableCfg::Ins().Pack(dbObj, p->dbObj, len))
	{
		L_ERROR("pack fail");
		return false;
	}
	msg.len = sizeof(ProtoType) + len;
	return m_con.SendData(msg);
}

bool db::BaseDbproxy::Query(const db::BaseTable &dbObj, uint32 limit_num/*=1*/)
{
	using ProtoType = query_cs;
	MsgPack msg;
	ProtoType *p = new (msg.data)ProtoType;
	p->limit_num = limit_num;

	//dbObj可用长度
	size_t len = ArrayLen(msg.data) - sizeof(ProtoType);
	if (!TableCfg::Ins().Pack(dbObj, p->dbObj, len))
	{
		L_ERROR("pack fail");
		return false;
	}
	msg.len = sizeof(ProtoType) + len;
	return m_con.SendData(msg);
}

void db::BaseDbproxy::ParseInsert(const proto::insert_sc *msg)
{
	//unpack insert_sc.dbObj
// BaseTable &t; get table obj
//t.tableId to concrete fun
	//BaseTable *pT;

	//using ComFun = void(bool isOk, const void *dbObj); //查询回调， 抽象类型。 
	//uint16_t cmdId = *(const uint16_t *)msg_pack.data;
	//ComFun *fun = (ComFun *)(m_Id2QueryCb[cmdId]);
	//(*fun)(this, msg_pack.data);

}

void db::BaseDbproxy::ParseQuery(const proto::query_sc *msg)
{

}

uint16_t db::DbClientCon::Send(proto::insert_cs &msg)
{
	return 0;
}
