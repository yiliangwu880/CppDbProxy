#pragma once
#include "base_include.h"
#include <unordered_map>

class InnerSvrCon : public lc::SvrCon
{
	std::unordered_map<uint16_t, void *> m_cmdId2Cb; //proto 消息ID 2 回调

private:
	virtual void OnRecv(const lc::MsgPack &msg_pack) override;
	virtual void OnConnected() override;

public:
	InnerSvrCon();

private:
	//注册 proto消息回调
	template<class ProtoMsg, class Fun>
	void RegProtoParse(Fun fun)
	{
		ProtoMsg t;
		if (m_cmdId2Cb.find(t.id) != m_cmdId2Cb.end())
		{
			L_ERROR("repeated reg");
			return;
		}
		m_cmdId2Cb[t.id] = (void *)fun;
	}

	//@len 表示 msg 有效长度
	static void ParseInsert(InnerSvrCon &con, const proto::insert_cs *msg, uint32_t len);
	static void ParseQuery(InnerSvrCon &con, const proto::query_cs *msg, uint32_t len);
	static void ParseUpdate(InnerSvrCon &con, const proto::update_cs *msg, uint32_t len);
	static void ParseDel(InnerSvrCon &con, const proto::del_cs *msg, uint32_t len);
};

class DbServer : public lc::Listener<InnerSvrCon>, public Singleton<DbServer>
{
public:
};
