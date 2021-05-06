#include "DbServer.h"
#include "svr_util/include/read_cfg.h"
#include "db_con.h"

using namespace std;
using namespace lc;
using namespace db;
using namespace su;
using namespace proto;

namespace
{
	//把 BaseTable对象 和 协议结构ProtoType  构建在 MsgPack,并设置MsgPack::len。ProtoType 其他字段内容未赋值。
	//类似下面的结构：dataLen data[0]必须最后定义，存放db 对象
	//	struct query_sc
	//{
	//	const uint16_t id = 6;
	//	bool ret;  //其他内容
	//  uint32_t dataLen;
	//	char data[0]; //一个db 对象
	//};
	template<class ProtoType>
	ProtoType *BuildMsgPack(MsgPack &msg, const db::BaseTable &data)
	{
		//要求dataLen data[0]必须最后定义
		static_assert((size_t)&(((ProtoType *)(nullptr))->dataLen) == sizeof(ProtoType) - sizeof(uint32_t));
		static_assert((size_t)&(((ProtoType *)(nullptr))->data) == sizeof(ProtoType));
		static_assert((size_t)&(((ProtoType *)(nullptr))->id) == 0); //id必须首先定义

		ProtoType *p = new (msg.data)ProtoType;
		//data可用长度
		size_t len = ArrayLen(msg.data) - sizeof(*p);
		if (!TableCfg::Ins().Pack(data, p->data, len))		{			L_ERROR("pack fail");			return nullptr;		}		p->dataLen = len;		msg.len = sizeof(ProtoType) + len;		return p;	}
}


InnerSvrCon::InnerSvrCon()
{
	RegProtoParse(ParseInsert);
	RegProtoParse(ParseQuery);
	RegProtoParse(ParseUpdate);
	RegProtoParse(ParseDel);
	RegProtoParse(ParseSql);
}

void InnerSvrCon::OnRecv(const MsgPack &msg)
{
	using ComFun = void(InnerSvrCon &con, const char &protoMsg); //消息回调函数， 抽象类型。 
	uint16_t cmdId = *(const uint16_t *)msg.data; //约定协议前 uint16_t 为 cmdId.  
	auto it = m_cmdId2Cb.find(cmdId);
	if (it == m_cmdId2Cb.end())
	{
		L_ERROR("unknow cmdId %d", cmdId);
		return;
	}
	ComFun *fun = (ComFun *)it->second;
	(*fun)(*this, *(const char *)(msg.data));
}

void InnerSvrCon::ParseInsert(InnerSvrCon &con, const proto::insert_cs &req)
{
	std::unique_ptr<BaseTable> pTable = TableCfg::Ins().Unpack(req.data, req.dataLen);
	L_COND_V(nullptr != pTable);
	BaseTable &data = *pTable;

	bool ret = DbConMgr::Ins().GetCon().Insert(data);
	MsgPack msg;
	insert_sc *rsp = BuildMsgPack<insert_sc>(msg, data);
	L_COND_V(rsp);
	rsp->ret = ret;
	con.SendData(msg);
}

void InnerSvrCon::ParseQuery(InnerSvrCon &con, const proto::query_cs &req)
{
	QueryResultRowCb cb = [&con](const db::BaseTable &data)
	{
		MsgPack msg;
		query_sc *rsp = BuildMsgPack<query_sc>(msg, data);
		L_COND_V(rsp);
		rsp->ret = true;
		con.SendData(msg);
	};
	if (req.isStr)
	{
		L_DEBUG("query by cond string, table_id= %d", req.table_id);
		string cond(req.data, req.dataLen);
		if (!DbConMgr::Ins().GetCon().Query(req.table_id, cond, req.limit_num, cb))
		{//fail response
			const Table *pTable = TableCfg::Ins().GetTable(req.table_id);
			L_COND_V(pTable);
			std::unique_ptr<BaseTable> pData =  pTable->factor();
			BaseTable &data = *pData;

			MsgPack msg;
			query_sc *rsp = BuildMsgPack<query_sc>(msg, data);
			L_COND_V(rsp);
			rsp->ret = false;
			con.SendData(msg);
		}
	}
	else
	{
		std::unique_ptr<BaseTable> pTable = TableCfg::Ins().Unpack(req.data, req.dataLen);
		L_COND_V(nullptr != pTable);
		BaseTable &data = *pTable;
		L_DEBUG("query by db obj value");
		if (!DbConMgr::Ins().GetCon().Query(data, req.limit_num, cb))
		{//fail response
			MsgPack msg;
			query_sc *rsp = BuildMsgPack<query_sc>(msg, data);
			L_COND_V(rsp);
			rsp->ret = false;
			con.SendData(msg);
		}
	}
}

void InnerSvrCon::ParseUpdate(InnerSvrCon &con, const proto::update_cs &req)
{
	std::unique_ptr<BaseTable> pTable = TableCfg::Ins().Unpack(req.data, req.dataLen);
	L_COND_V(nullptr != pTable);
	BaseTable &data = *pTable;

	DbConMgr::Ins().GetCon().Update(data);
}

void InnerSvrCon::ParseDel(InnerSvrCon &con, const proto::del_cs &req)
{
	std::unique_ptr<BaseTable> pTable = TableCfg::Ins().Unpack(req.data, req.dataLen);
	L_COND_V(nullptr != pTable);
	BaseTable &data = *pTable;

	bool ret = DbConMgr::Ins().GetCon().Del(data);
	MsgPack msg;
	del_sc *rsp = BuildMsgPack<del_sc>(msg, data);
	L_COND_V(rsp);
	rsp->ret = ret;
	con.SendData(msg);
}

void InnerSvrCon::ParseSql(InnerSvrCon &con, const proto::excute_sql_cs &req)
{
	string sql(req.data, req.dataLen);
	bool ret = DbConMgr::Ins().GetCon().ExecuteSql(sql);

	MsgPack msg;
	proto::excute_sql_sc *rsp = new (msg.data)proto::excute_sql_sc;	msg.len = sizeof(proto::excute_sql_sc);
	rsp->sql_id = req.sql_id;
	con.SendData(msg);
}

void InnerSvrCon::OnConnected()
{
	L_DEBUG("svr connect");
}


