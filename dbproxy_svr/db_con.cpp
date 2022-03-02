#include "db_con.h"
#include "mysql_con.h"

using namespace su;
using namespace lc;
using namespace std;
using namespace db;

bool DbConMgr::Init(const comCfg::S_dbproxy &cfg)
{
	
	//mysql_db Лђеп mongodb_db
	if (1)//(cfg.select_db == "mysql_db")
	{
		m_con = &MysqlCon::Ins();
	}
	//else if (cfg.select_db == "mongodb_db")
	//{
	//	return false;
	//	m_con = &MongodbConn::Ins();
	//}
	else
	{
		L_ERROR("cfg.select_db value is error");
		return false;
	}

	return m_con->ConnectDb(cfg);
}

IDbCon & DbConMgr::GetCon()
{
	if (nullptr == m_con)
	{
		L_FATAL("DbConMgr havn't init");
		L_ASSERT(false);
		return *(IDbCon*)nullptr; //cancel tsc tool warning
	}
	return *m_con;
}
