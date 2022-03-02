#include "base_include.h"
#include "svr_util/include/su_mgr.h"
#include "svr_util/include/single_progress.h"
#include "libevent_cpp/include/log_file.h"
#include <signal.h>
#include "DbServer.h"
#include "db_con.h"
#include "BaseAppMgr.h"
#include "DynamicCfg/comCfg.h"

using namespace su;
using namespace lc;
using namespace std;
using namespace db;

class AppMgr : public BaseAppMgr, public Singleton<AppMgr>
{
public:
	virtual bool OnStart() override
	{
		if (!CfgMgr::Ins().Init())
		{
			printf("read cfg fail!");
			return false;
		}

		 auto &cfg=CfgMgr::Ins().m_comCfg.dbproxy;
		if (!DbConMgr::Ins().Init(cfg))
		{
			return false;
		}
		L_INFO("dbproxy_svr svr addr:%s %d", cfg.ip.c_str(), cfg.port);
		return DbServer::Ins().Init(cfg.port, cfg.ip.c_str());

		return true;
	}
	virtual void OnExit() override
	{

	}
};


int main(int argc, char* argv[])
{
	AppMgr::Ins().Start(argc, argv, "dbproxy_svr");
	return 0;
}

