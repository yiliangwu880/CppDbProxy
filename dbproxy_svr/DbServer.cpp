#include "DbServer.h"
#include "svr_util/include/read_cfg.h"
#include "google/protobuf/message.h"
#include "base.pb.h"
#include "db_con.h"
#include "parser.h"

using namespace std;
using namespace lc;
using namespace google::protobuf;
using namespace db;

using UINT64 =::uint64;

namespace
{
	//简化解包操作。赋值并移动指针
	template<class T>
	void ParseCp(T &dst, const char *&src)
	{
		dst = *(T *)(src); // 类似 dst = *(uint32 *)(src)
		src = src + sizeof(dst);
	}
}

void InnerSvrCon::Handle_CMD_INIT_TABLE(const char *msg, uint16 msg_len)
{
	IDbCon &db_con = DbConMgr::Obj().GetCon();
	ReqInitTable req;
	L_COND(req.ParseFromArray(msg, msg_len), "msg parse fail");
	RspInitTable rsp;
	db_con.InitTable(req, rsp);
	Send(rsp);
}

void InnerSvrCon::Handle_CMD_INSERT(const char *msg, uint16 msg_len)
{
	IDbCon &db_con = DbConMgr::Obj().GetCon();
	ReqInsertData req;
	L_COND(req.ParseFromArray(msg, msg_len), "msg parse fail");
	UINT64 num_key;
	string str_key;
	bool r = db_con.Insert(req, num_key, str_key);
	RspInsertData rsp;
	rsp.set_msg_name(req.msg_name());
	rsp.set_num_key(num_key);
	rsp.set_str_key(str_key);
	rsp.set_is_ok(r);
	Send(rsp);
}

void InnerSvrCon::Handle_CMD_UPDATE(const char *msg, uint16 msg_len)
{
	IDbCon &db_con = DbConMgr::Obj().GetCon();
	ReqUpdateData req;
	L_COND(req.ParseFromArray(msg, msg_len), "msg parse fail");
	UINT64 num_key;
	string str_key;
	bool r = db_con.Update(req, num_key, str_key);
	RspUpdateData rsp;
	rsp.set_msg_name(req.msg_name());
	rsp.set_num_key(num_key);
	rsp.set_str_key(str_key);
	rsp.set_is_ok(r);
	Send(rsp);
}

void InnerSvrCon::Handle_CMD_GET(const char *msg, uint16 msg_len)
{
	IDbCon &db_con = DbConMgr::Obj().GetCon();
	ReqGetData req;
	L_COND(req.ParseFromArray(msg, msg_len), "msg parse fail");
	db_con.Get(req, *this);
}

void InnerSvrCon::Handle_CMD_DEL(const char *msg, uint16 msg_len)
{
	IDbCon &db_con = DbConMgr::Obj().GetCon();
	ReqDelData req;
	L_COND(req.ParseFromArray(msg, msg_len), "msg parse fail");

	RspDelData rsp;
	db_con.Del(req, rsp);
	Send(rsp);
}

void InnerSvrCon::Handle_CMD_SQL(const char *msg, uint16 msg_len)
{
	IDbCon &db_con = DbConMgr::Obj().GetCon();
	ReqSql req;
	L_COND(req.ParseFromArray(msg, msg_len), "msg parse fail");

	RspSql rsp;
	rsp.set_is_ok(db_con.ExecuteSql(req.exe_str()));
	Send(rsp);
}

void InnerSvrCon::OnRecv(const MsgPack &msg_pack)
{
	if (msg_pack.len < sizeof(db::Cmd))
	{
		L_WARN("illegal msg. msg.len=%d", msg_pack.len);
		return;
	}
	const char *cur = msg_pack.data;
	ParseCp(m_cur_cmd, cur);
	const char *msg = cur; 
	uint16 msg_len = msg_pack.len - sizeof(db::Cmd);
	switch (m_cur_cmd)
	{
	case CMD_INIT_TABLE: Handle_CMD_INIT_TABLE(msg, msg_len); return;
	case CMD_INSERT    : Handle_CMD_INSERT(msg, msg_len); return;
	case CMD_UPDATE    : Handle_CMD_UPDATE(msg, msg_len); return;
	case CMD_GET       : Handle_CMD_GET(msg, msg_len); return;
	case CMD_DEL       : Handle_CMD_DEL(msg, msg_len); return;
	case CMD_SQL       : Handle_CMD_SQL(msg, msg_len); return;
	default:
		L_WARN("illegal msg. cmd=%d", m_cur_cmd); return;
		break;
	}
}

void InnerSvrCon::OnConnected()
{
	L_DEBUG("svr connect");
}

void InnerSvrCon::Send(const google::protobuf::Message &msg)
{
	Send(m_cur_cmd, msg);
}

void InnerSvrCon::Send(db::Cmd cmd, const google::protobuf::Message &msg)
{
	string s;
	bool r = msg.SerializeToString(&s);
	L_COND(r, " msg.SerializeToString fail. cmd=%s", db::Cmd_Name(cmd).c_str());


	string msg_pack;
	msg_pack.append((const char *)&cmd, sizeof(cmd));
	msg_pack.append(s);
	SendPack(msg_pack);
}
