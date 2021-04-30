#include "base_include.h"
#include "svr_util/include/su_mgr.h"
#include "svr_util/include/single_progress.h"
#include "libevent_cpp/include/log_file.h"
#include <signal.h>
#include "DbServer.h"
#include "db_con.h"

using namespace su;
using namespace lc;
using namespace std;
using namespace db;

class MyLcLog : public lc::ILogPrinter, public Singleton<MyLcLog>
{
public:
	virtual void Printf(lc::LogLv lv, const char * file, int line, const char *fun, const char * pattern, va_list vp) override
	{
		su::LogMgr::Ins().Printf((su::LogLv)lv, file, line, fun, pattern, vp);
	}
};

//base application
class BaseApp
{
public:
	void Start(int argc, char* argv[], const string &app_name, bool is_daemon);
	virtual bool OnStart() = 0;
	void Stop();
private:
	static void OnExitProccess();
};



void BaseApp::Start(int argc, char* argv[], const string &app_name, bool is_daemon)
{
	if (is_daemon)
	{
		//当nochdir为0时，daemon将更改进城的根目录为root(“ / ”)。
		//当noclose为0是，daemon将进城的STDIN, STDOUT, STDERR都重定向到 / dev / null。
		if (0 != daemon(1, 0))
		{
			printf("daemon fail\n");
			return;
		}
	}
	SuMgr::Ins().Init();

	//start or stop proccess
	SingleProgress::Ins().Check(argc, argv, app_name.c_str(), is_daemon);

	lc::LogMgr::Ins().SetLogPrinter(MyLcLog::Ins());

	if (!OnStart())
	{
		L_INFO("start fail");
		return;
	}

	EventMgr::Ins().Dispatch();
	L_INFO("main end");
}


void BaseApp::Stop()
{
	EventMgr::Ins().StopDispatch();
}

void BaseApp::OnExitProccess()
{
	L_INFO("OnExitProccess");
	EventMgr::Ins().StopDispatch();
}

class MyApp: public BaseApp
{
public:
	virtual bool OnStart() override
	{
		const Cfg &cfg = CfgMgr::Ins().GetCfg();
		if (!DbConMgr::Ins().Init(cfg))
		{
			return false;
		}
		L_INFO("dbproxy_svr svr addr:%s %d", cfg.ip.c_str(), cfg.port);
		return DbServer::Ins().Init(cfg.port, cfg.ip.c_str());
	}

private:
};


int main(int argc, char* argv[])
{
	if (!CfgMgr::Ins().Init())
	{
		printf("read cfg fail!");
		return 0;
	}
	const Cfg &cfg = CfgMgr::Ins().GetCfg();

	MyApp app;
	app.Start(argc, argv, "dbproxy_svr", cfg.is_daemon);
	return 0;
}

