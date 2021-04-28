#include "db_con.h"
#include "parser.h"
#include "mysql_con.h"
#include "mongodb_con.h"

using namespace su;
using namespace lc;
using namespace std;
using namespace google::protobuf;
using namespace db;

DbConMgr::~DbConMgr()
{
	if (nullptr != m_con)
	{
		delete m_con;
		m_con = nullptr;
	}
}

bool DbConMgr::Init(const Cfg &cfg)
{
	//mysql_db Лђеп mongodb_db
	if (cfg.select_db == "mysql_db")
	{
		MysqlCon *con = new MysqlCon;
		m_con = con;
		return con->ConnectDb(cfg);
	}
	else if (cfg.select_db == "mongodb_db")
	{
		return false;
		MongodbConn *con = new MongodbConn;
		m_con = con;
		return con->ConnectDb(cfg);
	}
	else
	{
		L_ERROR("cfg.select_db value is error");
		return false;
	}

	return true;
}

IDbCon & DbConMgr::GetCon()
{
	if (nullptr == m_con)
	{
		L_FATAL("DbConMgr havn't init");
		L_ASSERT(false);
	}
	return *m_con;
}
