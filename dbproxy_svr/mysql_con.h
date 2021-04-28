/*
 管理连接db客户端
*/

#pragma once
#include "base_include.h"
#include "db_con.h"
#include "mysql_connection.h"
#include "cppconn/resultset.h"

class MysqlCon : public IDbCon
{
private:
	sql::Connection* m_con = nullptr;

public:
	virtual ~MysqlCon();

	virtual bool ConnectDb(const Cfg &cfg) override;

	virtual bool InitTable(const db::ReqInitTable &req, db::RspInitTable &rsp) override; //创建表， 检查表是否非法
	virtual bool Insert(const db::ReqInsertData &req, ::uint64 &num_key, std::string &str_key) override;
	virtual bool Update(const db::ReqUpdateData &req, ::uint64 &num_key, std::string &str_key) override;
	virtual bool Get(const db::ReqGetData &req, InnerSvrCon &con) override;
	virtual bool Del(const db::ReqDelData &req, db::RspDelData &rsp) override;
	virtual bool ExecuteSql(const std::string &sql_str) override;

private:
	void SetInsertPreparePara(sql::PreparedStatement &pstmt, google::protobuf::Message &msg);
	void SetUpdatePreparePara(sql::PreparedStatement &pstmt, google::protobuf::Message &msg);
	bool SetField(google::protobuf::Message& msg, const google::protobuf::FieldDescriptor &field, const sql::ResultSet& res);
	
	bool TryCreateTableSql(const std::string &msg_name, std::string &sql_str);
	bool CreateInsertSql(const google::protobuf::Message &msg, std::string &sql_str);
	bool CreateUpdateSql(const google::protobuf::Message &msg, std::string &sql_str);
	void CreateSelectSql(const db::ReqGetData &req, const std::string &table_name, std::string &sql_str);
	std::string GetCreateTypeStr(google::protobuf::FieldDescriptor::Type t, bool is_unique);

};