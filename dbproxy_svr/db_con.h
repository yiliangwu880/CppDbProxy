/*
 管理连接db客户端,抽象成，兼容mysql， mongodb
*/

#pragma once
#include "base_include.h"

using QueryResultRowCb = std::function<void(db::BaseTable &data)>;
class InnerSvrCon;
class IDbCon
{
public:
	virtual ~IDbCon() {};

	virtual bool ConnectDb(const Cfg &cfg) { return false; };

	virtual bool InitTable() { return false; }; //创建表， 检查表是否非法
	virtual bool Insert(const db::BaseTable &req) { return false; };
	virtual bool Update(const db::BaseTable &req) { return false; };
	virtual bool Query(const db::BaseTable &req, uint16_t limit_num, QueryResultRowCb cb) { return false; };
	virtual bool Del(const db::BaseTable &req) { return false; };

};

//管理连接db客户端
class DbConMgr : public Singleton<DbConMgr>
{

public:
	bool Init(const Cfg &cfg);

	IDbCon &GetCon();

private:
	IDbCon *m_con=nullptr;
};


