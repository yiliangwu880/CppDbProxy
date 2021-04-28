#pragma once
#include "base_include.h"

class InnerSvrCon : public lc::SvrCon
{
	db::Cmd m_cur_cmd=db::CMD_NONE;
public:
	InnerSvrCon(){}
	void Handle_CMD_INIT_TABLE(const char *msg, uint16 msg_len);
	void Handle_CMD_INSERT(const char *msg, uint16 msg_len);
	void Handle_CMD_UPDATE(const char *msg, uint16 msg_len);
	void Handle_CMD_GET(const char *msg, uint16 msg_len);
	void Handle_CMD_DEL(const char *msg, uint16 msg_len);
	void Handle_CMD_SQL(const char *msg, uint16 msg_len);

	void Send(db::Cmd cmd, const google::protobuf::Message &msg);
	//用最近接收消息号，发送
	void Send(const google::protobuf::Message &msg);
private:
	virtual void OnRecv(const lc::MsgPack &msg_pack) override;
	virtual void OnConnected() override;

};

class DbServer : public lc::Listener<InnerSvrCon>, public Singleton<DbServer>
{
public:
};
