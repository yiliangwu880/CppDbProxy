
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
#include "log_def.h"

namespace db {
	class Dbproxy;
	class DbClientCon;

	class DbClientCon : public lc::ClientCon, public Singleton<DbClientCon>
	{
	private:
		lc::Timer m_recon_tm;
	public:
		virtual void OnRecv(const lc::MsgPack &msg) override final;
		virtual void OnConnected() override final;
		virtual void OnDisconnected() override final;

	private:
		void OnTryReconTimeOut();
	};

}//namespace acc