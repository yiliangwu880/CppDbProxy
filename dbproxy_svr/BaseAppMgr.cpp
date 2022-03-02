#include "BaseAppMgr.h"
#include "su_mgr.h"
#include "single_progress.h"
#include "libevent_cpp/include/include_all.h"
#include "libevent_cpp/include/http.h"

using namespace su;
using namespace lc;
using namespace std;

class MyLcLog : public lc::ILogPrinter, public Singleton<MyLcLog>
{
public:
	virtual void Printf(lc::LogLv lv, const char * file, int line, const char *fun, const char * pattern, va_list vp) override
	{
		su::LogMgr::Ins().Printf((su::LogLv)lv, file, line, fun, pattern, vp);
	}
};


void BaseAppMgr::Start(int argc, char* argv[], const string &app_name)
{
	bool isDaemon = false;
	if (argc >= 2 && string("d") == argv[1])
	{
		isDaemon = true;
	}
	SingleProgress::Ins().Check(argc, argv, app_name.c_str(), isDaemon); //启动关闭进程管理
	su::LogMgr::Ins().DefaultFileName("log_" + app_name + ".txt");
	lc::LogMgr::Ins().SetLogPrinter(MyLcLog::Ins());
	SuMgr::Ins().Init();
	lc::Timer timer; //一个进程只需要一个lc::Timer，其他定时器由  svr_util 驱动
	auto f = std::bind(&BaseAppMgr::OnTimer, this);
	timer.StartTimer(30, f, true);
	L_COND_V(OnStart());
	EventMgr::Ins().Dispatch();
	L_INFO("main end");
}


void BaseAppMgr::AddPost(PostFun cb)
{
	m_postCb.push_back(cb);
}

void BaseAppMgr::OnTimer()
{
	{//post cb
		if (!m_postCb.empty())
		{
			vector<PostFun> tmp;
			swap(tmp, m_postCb);
			for (PostFun &cb : tmp)
			{
				cb();
			}
		}
	}
	if (SingleProgress::Ins().IsExit())
	{
		L_INFO("OnExitProccess");
		OnExit();
		EventMgr::Ins().StopDispatch();
		return;
	}
	SuMgr::Ins().OnTimer();
}
