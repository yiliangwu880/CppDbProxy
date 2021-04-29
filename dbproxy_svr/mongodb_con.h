
#pragma once
#include "base_include.h"
#include "db_con.h"

class MongodbConn : public IDbCon, public Singleton<MongodbConn>
{
public:
	virtual bool ConnectDb(const Cfg &cfg);
};
