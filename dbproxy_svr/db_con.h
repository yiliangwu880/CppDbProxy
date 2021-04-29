/*
 管理连接db客户端,抽象成，兼容mysql， mongodb
*/

#pragma once
#include "base_include.h"

class InnerSvrCon;
class IDbCon
{
public:
	virtual ~IDbCon() {};

	virtual bool ConnectDb(const Cfg &cfg) { return false; };

	virtual bool InitTable() ; //创建表， 检查表是否非法
	virtual bool Insert(const db::BaseTable &req);
	virtual bool Update(const db::BaseTable &req);
	virtual bool Query(const db::BaseTable &req, QueryResultRowCb cb)	;
	virtual bool Del(const db::BaseTable &req)	 ;

};

//管理连接db客户端
class DbConMgr : public Singleton<DbConMgr>
{

public:
	DbConMgr() {}
	~DbConMgr();
	bool Init(const Cfg &cfg);

	IDbCon &GetCon();

private:
	IDbCon *m_con=nullptr;
};


