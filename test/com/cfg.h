#pragma once
#include <string>
#include "libevent_cpp/include/include_all.h"
#include "svr_util/include/su_mgr.h"
#include "svr_util/include/single_progress.h"
#include "svr_util/include/read_cfg.h"
#include "unit_test.h"

using namespace su;
using namespace lc;

class CfgMgr : public Singleton<CfgMgr>
{
public:
	bool Init(const char *path_name = nullptr)
	{
		UNIT_INFO("init cfg");
		su::Config cfg;
		if (nullptr == path_name)
		{
			path_name = "./cfg.txt";
		}
		UNIT_ASSERT(cfg.init(path_name));

		std::string ip = "127.0.0.1";
		dbproxy_svr_ip =cfg.GetStr("dbproxy_svr_ip");
		dbproxy_svr_port =cfg.GetInt("dbproxy_svr_port");
		is_daemon =cfg.GetInt("is_daemon");
		return true;
	}

public:
	std::string dbproxy_svr_ip;
	uint16 dbproxy_svr_port;
	bool is_daemon;
};
