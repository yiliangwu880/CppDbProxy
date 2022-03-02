/*
 管理连接db客户端
*/

#pragma once
#include "base_include.h"
#include "db_con.h"
#include "mysql_connection.h"
#include "cppconn/resultset.h"

class MysqlCon : public IDbCon, public Singleton<MysqlCon>
{
private:
	sql::Connection* m_con = nullptr;

public:
	virtual ~MysqlCon();

	virtual bool ConnectDb(const comCfg::S_dbproxy &cfg) override;

	virtual bool InitTable() override; //创建表， 检查表是否非法
	virtual bool Insert(const db::BaseTable &data)override;
	virtual bool Update(const db::BaseTable &data)override;
	virtual bool Query(const db::BaseTable &data, uint32_t limit_num, QueryResultRowCb cb)override;
	virtual bool Query(uint16_t table_id, std::string &cond, uint32_t limit_num, QueryResultRowCb cb) override;
	virtual bool Del(const db::BaseTable &data)	  override;
	virtual bool ExecuteSql(const std::string &req) override;
private:
	bool SetField(db::BaseTable &data, const db::Field &field, const sql::ResultSet& res);

	bool TryCreateTableSql(const db::Table &table, std::string &sql_str);
	bool CreateInsertSql(const db::BaseTable &data, std::string &sql_str);
	bool CreateUpdateSql(const db::BaseTable &data, std::string &sql_str);
	void CreateSelectSql(const db::BaseTable &data, const std::string &table_name, uint32_t limit_num, std::string &sql_str);
	void CreateSelectSql(const std::string &cond, const std::string &table_name, uint32_t limit_num, std::string &sql_str);

	std::string GetCreateTypeStr(db::FieldType t, bool is_unique);
};