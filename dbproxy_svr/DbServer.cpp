#include "DbServer.h"
#include "svr_util/include/read_cfg.h"
#include "db_con.h"
#include "parser.h"

using namespace std;
using namespace lc;
using namespace db;
using namespace proto;

namespace
{
	//把 BaseTable对象 和 协议结构  构建在 MsgPack。ProtoType 其他字段内容未赋值。
	//类似下面的结构：data[0]最后定义，存放db 对象
	//	struct query_sc
	//{
	//	const uint16_t id = 6;
	//	bool ret;
	//	char data[0]; //一个db 对象
	//};
	template<class ProtoType>
	ProtoType *BuildMsgPack(MsgPack &msg, db::BaseTable &data)
	{
		ProtoType *p = new (msg.data)ProtoType;
		//data可用长度
		size_t len = ArrayLen(msg.data) - sizeof(*p);
		if (!TableCfg::Ins().Pack(data, p->data, len))		{
			L_ERROR("pack fail");			return nullptr;		}		msg.len = sizeof(ProtoType) + len;	}
}


InnerSvrCon::InnerSvrCon()
{
	RegProtoParse<insert_cs>(&BaseDbproxy::ParseInsert);
	RegProtoParse<query_cs>(&BaseDbproxy::ParseQuery);
	RegProtoParse<update_cs>(&BaseDbproxy::ParseUpdate);
	RegProtoParse<del_cs>(&BaseDbproxy::ParseDel);
}

void InnerSvrCon::OnRecv(const MsgPack &msg)
{
	using ComFun = void(InnerSvrCon &con, const void *protoMsg, decltype(msg.len)); //消息回调函数， 抽象类型。 
	uint16_t cmdId = *(const uint16_t *)msg.data; //约定协议前 uint16_t 为 cmdId.  
	ComFun *fun = (ComFun *)(m_cmdId2Cb[cmdId]);
	(*fun)(*this, msg.data, msg.len);
}

void InnerSvrCon::ParseQuery(InnerSvrCon &con, const proto::query_cs *req, uint32_t req_len)
{
	L_COND_V(req_len >= sizeof(req));
	std::unique_ptr<BaseTable> pTable = TableCfg::Ins().Unpack(req->data, req_len - sizeof(req));
	L_COND_V(nullptr != pTable);
	BaseTable &data = *pTable;
	IDbCon &dbCon = DbConMgr::Ins().GetCon();

	QueryResultRowCb cb = [](db::BaseTable &data)
	{
		MsgPack msg;
		query_sc *rsp = BuildMsgPack<query_sc>(msg, data);
		L_COND_V(rsp);
		rsp->ret = true;
		return SendData(msg);
	};
	if (!dbCon.Query(data, cb))
	{//response
		MsgPack msg;
		query_sc *rsp = BuildMsgPack<query_sc>(msg, data);
		L_COND_V(rsp);
		rsp->ret = false;
		return SendData(msg);
	}
}

void InnerSvrCon::OnConnected()
{
	L_DEBUG("svr connect");
}


