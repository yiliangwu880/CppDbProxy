#include "db_driver.h"
#include "connect.h"
#include "log_def.h"

using namespace std;
using namespace lc;
using namespace su;
using namespace db;
using namespace proto;

static const uint32 RE_CON_INTERVAL_SEC = 10; //x秒尝试重连

db::DbClientCon::DbClientCon()
{
	RegParse<insert_sc>(&BaseDbproxy::ParseInsert);
	RegParse<query_sc>(&BaseDbproxy::ParseQuery);
}

void DbClientCon::OnRecv(const lc::MsgPack &msg_pack)
{
	using ComFun = void(const void *msg); //消息回调函数， 抽象类型。 
	uint16_t cmdId = *(const uint16_t *)msg_pack.data;
	ComFun *fun = (ComFun *)(m_cmdId2Cb[cmdId]);
	(*fun)(msg_pack.data);
}


void db::DbClientCon::OnTryReconTimeOut()
{
	TryReconnect();
}

void DbClientCon::OnConnected()
{
	if (BaseDbproxy::Ins().m_conCb)
	{
		BaseDbproxy::Ins().m_conCb();
	}
}

void DbClientCon::OnDisconnected()
{
	L_DEBUG("start try recon timer , sec=%d", RE_CON_INTERVAL_SEC);
	auto f = std::bind(&DbClientCon::OnTryReconTimeOut, this);
	m_recon_tm.StopTimer();
	m_recon_tm.StartTimer(RE_CON_INTERVAL_SEC*1000, f);
}


