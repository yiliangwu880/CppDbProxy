//驱动具体实现

#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <map>
#include "libevent_cpp/include/include_all.h"
#include "svr_util/include/singleton.h"
#include "svr_util/include/easy_code.h"
#include "svr_util/include/typedef.h"
#include "db_driver.h"
#include "proto/proto.h"

namespace db {
	class BaseDbproxy;
	class DbClientCon;
	static const size_t MAX_SEND_LEN = 1024 * 1024 * 10; //最大发送长度 10 M

	class DbClientCon : public lc::ClientCon
	{
	private:

		lc::Timer m_recon_tm;
		std::unordered_map<uint16_t, void *> m_cmdId2Cb; //消息ID 2 回调
	public:
		DbClientCon();
		virtual void OnRecv(const lc::MsgPack &msg) override final;
		virtual void OnConnected() override final;
		virtual void OnDisconnected() override final;

		//template<>
		//成功返回 session id
		uint16_t Send(proto::insert_cs &msg);

		template<class ProtoMsg, class Fun>
		void RegParse(Fun fun)
		{
			ProtoMsg t;
			m_cmdId2Cb[t.id] = (void *)fun;
		}
	private:
		void OnTryReconTimeOut();
	};

}//namespace acc