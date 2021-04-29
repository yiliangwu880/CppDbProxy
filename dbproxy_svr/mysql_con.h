/*
 管理连接db客户端
*/

#pragma once
#include "base_include.h"
#include "db_con.h"
#include "mysql_connection.h"
#include "cppconn/resultset.h"

using QueryResultRowCb = std::function<void(db::BaseTable &data)>;
class MysqlCon : public IDbCon, Singleton<MysqlCon>
{
private:
	sql::Connection* m_con = nullptr;

public:
	virtual ~MysqlCon();

	virtual bool ConnectDb(const Cfg &cfg) override;

	virtual bool InitTable() override; //创建表， 检查表是否非法
	virtual bool Insert(const db::BaseTable &req)override;
	virtual bool Update(const db::BaseTable &req)override;
	virtual bool Query(const db::BaseTable &req, QueryResultRowCb cb)override;
	virtual bool Del(const db::BaseTable &req)	  override;

public:
	bool TryCreateSchema(const std::string &name);


private:
	void SetUpdatePreparePara(sql::PreparedStatement &pstmt, const db::BaseTable &data);
	bool SetField(BaseTable &data, const Field &field, const sql::ResultSet& res);

	bool TryCreateTableSql(const db::Table &table, std::string &sql_str);
	bool CreateInsertSql(const db::BaseTable &data, std::string &sql_str);
	bool CreateUpdateSql(const db::BaseTable &data, std::string &sql_str);
	void CreateSelectSql(const db::BaseTable &data, const std::string &table_name, uint16_t limit_num, std::string &sql_str);
	std::string GetCreateTypeStr(db::FieldType t, bool is_unique);
};