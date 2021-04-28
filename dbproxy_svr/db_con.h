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

	virtual bool InitTable(const db::ReqInitTable &req, db::RspInitTable &rsp) {return false;}; //创建表， 检查表是否非法
	virtual bool Insert(const db::ReqInsertData &req, ::uint64 &num_key, std::string &str_key) { return false; };
	virtual bool Update(const db::ReqUpdateData &req, ::uint64 &num_key, std::string &str_key) { return false; };
	virtual bool Get(const db::ReqGetData &req, InnerSvrCon &con) { return false; };
	virtual bool Del(const db::ReqDelData &req, db::RspDelData &rsp) { return false; };
	virtual bool ExecuteSql(const std::string &sql_str) { return false; };

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


