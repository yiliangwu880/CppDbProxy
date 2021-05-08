#include "db_con.h"
#include <cppconn/driver.h>
#include <cppconn/datatype.h>
#include <mysql_driver.h>
#include <cppconn/prepared_statement.h>
#include "mysql_con.h"
#include "svr_util/include/str_util.h"
#include "DbServer.h"
#include <iostream>
#include <iterator>  

using namespace su;
using namespace lc;
using namespace std;
using namespace db;

namespace
{
	//有值内容构建条件字符串，输入例如：“serverid = 201 and job = 1 and account = '1'”
	string BuildSelectCond(const db::BaseTable &data)
	{
		string str;
		const Table *table = TableCfg::Ins().GetTable(data.TableId());
		L_COND(table, "");

		int idx = 0;
		for (const Field &field : table->m_vecField)
		{
			uint8_t *pField = (uint8_t*)&data + field.pOffset;
			if (field.type == FieldType::t_bytes || field.type == FieldType::t_string)
			{
				std::string *pStr = (std::string *)pField;
				if (pStr->empty())
				{
					continue;
				}
			}
			else if (field.type == FieldType::t_struct)
			{
				continue;
			}
			else
			{
				static uint64_t zero = uint64_t();
				if (0 == memcmp(pField, &zero, field.fieldSize))
				{
					continue;
				}
			}
			if (idx > 0 )
			{
				str += " and ";
			}
			switch (field.type)
			{
			case FieldType::t_bytes:
			case FieldType::t_string:
				str += field.name + " = " + *(std::string *)pField; //todo :blob内容，还没解决方案。
				break;
#define EASY_CODE(fieldType)\
			case FieldType::t_##fieldType:\
				str += field.name + " = " + StrNum::NumToStr(*(RealTypeTraits<FieldType::t_##fieldType>::Type *)pField);\
				break;\

					EASY_CODE(int32_t)
					EASY_CODE(uint32_t)
					EASY_CODE(int64_t)
					EASY_CODE(uint64_t)
					EASY_CODE(double)

#undef EASY_CODE
			default:
				L_COND(table, "");
				break;
			}
			idx++;
		}
		return str;
	}

	void SetFieldParam(const db::BaseTable &data, const Field &field, sql::PreparedStatement &pstmt, int idx)
	{
		char *pField = (char*)&data + field.pOffset;
		switch (field.type)
		{
		default:
			L_ERROR("unknow type %d", (int)field.type);
			break;
		case FieldType::t_int32_t:
			pstmt.setInt(idx, *(int32_t *)pField);
			break;
		case FieldType::t_uint32_t:
			pstmt.setUInt(idx, *(uint32_t *)pField);
			break;
		case FieldType::t_int64_t:
			pstmt.setInt64(idx, *(int64_t *)pField);
			break;
		case FieldType::t_uint64_t:
			pstmt.setUInt64(idx, *(uint64_t *)pField);
			break;
		case FieldType::t_double:
			pstmt.setDouble(idx, *(double *)pField);
			break;
		case FieldType::t_bool:
			pstmt.setBoolean(idx, *(bool *)pField);
			break;
		case FieldType::t_string:
		case FieldType::t_bytes:
			pstmt.setString(idx, *(std::string *)pField);
			break;
		case FieldType::t_struct:
			std::string str;
			field.Pack(data, str);
			pstmt.setString(idx, str);
			break;
		}
	}

}

void MysqlCon::CreateSelectSql(const db::BaseTable &data, const string &table_name, uint32_t limit_num, string &sql_str)
{
	//SELECT t.*
	//	FROM GameDB.players t
	//	WHERE serverid = 201 and job = 1 and account = '1'
	//	LIMIT 5
	sql_str = "SELECT ";
	sql_str += "* FROM ";
	sql_str += table_name;
	string cond = BuildSelectCond(data);
	if (!cond.empty())
	{
		sql_str += " WHERE " + cond;
	}
	if (limit_num != 0)
	{
		sql_str += " limit ";
		sql_str += StrNum::NumToStr(limit_num);
	}
}
void MysqlCon::CreateSelectSql(const std::string &cond, const string &table_name, uint32_t limit_num, string &sql_str)
{
	//SELECT t.*
	//	FROM GameDB.players t
	//	WHERE serverid = 201 and job = 1 and account = '1'
	//	LIMIT 5
	sql_str = "SELECT ";
	sql_str += "* FROM ";
	sql_str += table_name;
	if (!cond.empty())
	{
		sql_str += " WHERE " + cond;
	}
	if (limit_num != 0)
	{
		sql_str += " limit ";
		sql_str += StrNum::NumToStr(limit_num);
	}
}
bool MysqlCon::InitTable()
{
	L_COND_F(m_con);
	for (auto &v : TableCfg::Ins().GetAllTable())
	{
		const Table &table = v.second;
		try {
			string sql_str;
			L_DEBUG("TryCreateTableSql msg_name=%s", table.name.c_str());
			if (!TryCreateTableSql(table, sql_str))
			{
				L_ERROR("CreateSql fail");
				return false;
			}

			L_DEBUG("create sql=%s", sql_str.c_str());
			std::unique_ptr<sql::PreparedStatement> pstmt(m_con->prepareStatement(sql_str));
			int affect_row = pstmt->executeUpdate();
			if (0 != affect_row)
			{
				L_ERROR("create table fail. affect_row=%d, table name[%s], sql[%s]", affect_row, table.name.c_str(), sql_str.c_str());
				return false;
			}
		}
		catch (sql::SQLException &e) {
			L_ERROR("%s, MySQL error code:%d, SQLState:%s", e.what(), e.getErrorCode(), e.getSQLStateCStr());
			return false;
		}
	}
	return true;
}

bool MysqlCon::Insert(const db::BaseTable &data)
{
	L_COND_F(m_con);
	const Table *table = TableCfg::Ins().GetTable(data.TableId());
	L_COND_F(table);
	try {
		string sql_str;
		CreateInsertSql(data, sql_str);

		L_DEBUG("create insert sql=%s", sql_str.c_str());
		unique_ptr< sql::PreparedStatement > pstmt(m_con->prepareStatement(sql_str));
		int idx = 1;
		for (const Field &field : table->m_vecField)
		{
			SetFieldParam(data, field, *pstmt, idx);
			idx++;
		}
		int affect_row = pstmt->executeUpdate();
		if (1 != affect_row)
		{
			L_ERROR("insert table fail. row=%d, table name[%s]", affect_row, table->name.c_str());
			return false;
		}
		return true;
	}
	catch (sql::SQLException &e) {
		L_ERROR("%s, MySQL error code:%d, SQLState:%s", e.what(), e.getErrorCode(), e.getSQLStateCStr());
		return false;
	}
}

bool MysqlCon::Update(const db::BaseTable &data)
{
	const Table *table = TableCfg::Ins().GetTable(data.TableId());
	L_COND_F(table);
	L_COND_F(m_con);
	try {
		string sql_str;
		if (!CreateUpdateSql(data, sql_str))
		{
			L_ERROR("create update sql fail");
			return false;
		}
		L_DEBUG("create update sql=%s", sql_str.c_str());
		unique_ptr< sql::PreparedStatement > pstmt(m_con->prepareStatement(sql_str));
		int idx = 1;
		for (const Field &field : table->m_vecField)
		{
			uint8_t *pField = (uint8_t*)&data + field.pOffset;
			if (KeyType::MAIN == field.keyType)
			{
				continue;
			}
			SetFieldParam(data, field, *pstmt, idx);
			idx++;
		}
		int affect_row = pstmt->executeUpdate();
		if (1 != affect_row)
		{
			L_ERROR("update table fail. row=%d, table name[%s]", affect_row, table->name.c_str());
			return false;
		}
		return true;
	}
	catch (sql::SQLException &e) {
		L_ERROR("%s, MySQL error code:%d, SQLState:%s", e.what(), e.getErrorCode(), e.getSQLStateCStr());
		return false;
	}
}




MysqlCon::~MysqlCon()
{
	if (nullptr != m_con)
	{
		delete m_con;
		m_con = nullptr;
	}
}

bool MysqlCon::ConnectDb(const Cfg &cfg)
{
	L_COND_F(nullptr == m_con);
	try {
		auto mysql_db = cfg.mysql_db;
		sql::ConnectOptionsMap connection_properties;
		connection_properties["hostName"]      = mysql_db.db_ip;
		connection_properties["port"]          = mysql_db.db_port;
		connection_properties["userName"]      = mysql_db.db_user;
		connection_properties["password"]      = mysql_db.db_psw;
		connection_properties["OPT_RECONNECT"] = true;

		L_DEBUG("try connect mysql db[%s].\n %s %d this may need a few minute!", mysql_db.db_user.c_str()
			, mysql_db.db_ip.c_str(), mysql_db.db_port);
		sql::Driver* driver = sql::mysql::get_driver_instance();
		m_con = driver->connect(connection_properties);
		m_con->setSchema(mysql_db.db_name);
		L_DEBUG("connect mysql db ok");
		InitTable();
		return true;
	}
	catch (sql::SQLException &e) {
		L_ERROR("%s, MySQL error code:%d, SQLState:%s", e.what(), e.getErrorCode(), e.getSQLStateCStr());
		return false;
	}
}

bool MysqlCon::TryCreateTableSql(const db::Table &table, std::string &sql_str)
{
	sql_str = "CREATE TABLE IF NOT EXISTS `";
	sql_str += table.name;
	sql_str += "` (";

	string main_key;
	vector<string> index_key;
	for (const db::Field &field : table.m_vecField)
	{
		const std::string &name = field.name;
		//每个域 字符串模板 = "`a` varchar(11) NOT NULL,"
		sql_str += "`";
		sql_str += name;
		sql_str += "` ";

		bool is_unique = false;
		{
			if (db::KeyType::MAIN == field.keyType)
			{
				main_key = name;
				is_unique = true;
			}
			else if (db::KeyType::INDEX == field.keyType)
			{
				index_key.push_back(name);
				is_unique = true;
			}
		}
		sql_str += GetCreateTypeStr(field.type, is_unique);
		sql_str += ",";

	}
	if (main_key.empty())
	{
		L_ERROR("miss main key. msg name=%s", table.name.c_str());
		return false;
	}
	sql_str += "PRIMARY KEY(`" + main_key + "`)";
	for (const string &str : index_key)
	{
		sql_str += ",UNIQUE KEY `" + str + "` (`" + str + "`)";
	}
	sql_str += ") ENGINE = InnoDB DEFAULT CHARSET = utf8;";
	return true;
}


bool MysqlCon::SetField(BaseTable &data, const Field &field, const sql::ResultSet& res)
{
	uint8_t *pField = (uint8_t*)&data + field.pOffset;

	if (res.isNull(field.name))
	{
		return true;
	}
	try {
		switch (field.type)
		{
		default:
			L_ERROR("unknow type %d", (int)field.type);
			return false;
		case FieldType::t_double:
			*(double *)pField = res.getDouble(field.name);
			break;
		case FieldType::t_int32_t:
			*(int32_t *)pField = res.getInt(field.name);
			break;
		case FieldType::t_uint32_t:
			*(uint32_t *)pField = res.getUInt(field.name);
			break;
		case FieldType::t_int64_t:
			*(int64_t *)pField = res.getInt64(field.name);
			break;
		case FieldType::t_uint64_t:
			*(uint64_t *)pField = res.getUInt64(field.name);
			//L_DEBUG("table id,name %d %s v=%ld", data.TableId(), field.name.c_str(), *(uint64_t *)pField);
			break;
		case FieldType::t_bool:
			*(bool *)pField = res.getBoolean(field.name);
			break;
		case FieldType::t_string:
			*(string *)pField = res.getString(field.name).asStdString();
			break;
		case FieldType::t_bytes:
		{
			unique_ptr<std::istream> inStream(res.getBlob(field.name));
			if (nullptr == inStream)
			{
				L_ERROR("res.getBlob() fail field.name()=%s", field.name.c_str());
				return false;
			}
			string &str = *(string *)pField;
			str.assign(std::istreambuf_iterator<char>{*inStream}, std::istreambuf_iterator<char>{});
		}
			break;
		case FieldType::t_struct:
		{
			unique_ptr<std::istream> inStream(res.getBlob(field.name));
			if (nullptr == inStream)
			{
				L_ERROR("res.getBlob() fail field.name()=%s", field.name.c_str());
				return false;
			}
			std::string str;
			str.assign(std::istreambuf_iterator<char>{*inStream}, std::istreambuf_iterator<char>{});
			field.Unpack(data, str);
		}
			break;
		}
	}
	catch (sql::InvalidArgumentException e) {
		//结果集中没有改字段，不进行设置
		L_ERROR("can not set filed %s, ", field.name.c_str());
		L_ERROR("%s, MySQL error code:%d, SQLState:%d", e.what(), e.getErrorCode(), e.getSQLStateCStr());
		return false;
	}
	catch (sql::SQLException &e) {
		L_ERROR("%s, MySQL error code:%d, SQLState:%s", e.what(), e.getErrorCode(), e.getSQLStateCStr());
		return false;
	}
	return true;
}



bool MysqlCon::CreateUpdateSql(const db::BaseTable &data, std::string &sql_str)
{
	const Table *table = TableCfg::Ins().GetTable(data.TableId());
	L_COND(table, false);

	sql_str += "UPDATE ";
	sql_str += table->name;
	sql_str += " SET ";

	bool need_comma = false;
	string str_key;
	uint64_t num_key = 0; 
	const Field *key_field = nullptr;//主键描述
	for (const Field &field : table->m_vecField)
	{
		uint8_t *pField = (uint8_t*)&data + field.pOffset;
		if (KeyType::MAIN == field.keyType)
		{
			if (FieldType::t_string == field.type)
			{
				str_key = *(string *)pField;
			}
			else if(FieldType::t_uint64_t == field.type || FieldType::t_int64_t == field.type)
			{
				num_key = *(uint64_t *)pField;
			}
			else
			{
				//暂时不开发其他类型主键
				L_ERROR("main key must be uint64");
				return false;
			}
			key_field = &field;
			continue;
		}
		if (need_comma)
		{
			sql_str += ",";
		}
		sql_str += field.name;
		sql_str += "=?";
		need_comma = true;
	}	
	L_COND(key_field, false);
	sql_str += " ";
	char where[1000]; 
	if (0 == num_key)
	{
		snprintf(where, sizeof(where), "WHERE %s='%s'  limit 1", key_field->name.c_str(), str_key.c_str());
	}
	else //其他一致认为是uint64处理
	{
		snprintf(where, sizeof(where), "WHERE %s=%lu  limit 1", key_field->name.c_str(), num_key);
	}
	sql_str += where;

	sql_str += ";";
	return true;
}

bool MysqlCon::Query(const db::BaseTable &data, uint32_t limit_num, QueryResultRowCb cb)
{
	L_COND_F(m_con);
	const Table *table = TableCfg::Ins().GetTable(data.TableId());
	L_COND_F(table);

	try {
		string sql_str;
		CreateSelectSql(data, table->name, limit_num, sql_str);
		L_DEBUG("select sql [%s]", sql_str.c_str());
		unique_ptr< sql::Statement > stmt(m_con->createStatement());
		stmt->execute(sql_str);

		int row_num = 0;
		do
		{
			unique_ptr<sql::ResultSet> ret(stmt->getResultSet());
			if (0 == row_num && nullptr == ret) //一个数据都没有
			{
				L_ERROR("execute sql fail [%s]", sql_str.c_str());
				return false;
			}
			while (ret->next()) {
				unique_ptr<db::BaseTable> pData = table->factor();
				for (const Field &field : table->m_vecField)
				{
					if (!SetField(*pData, field, *ret))
					{
						L_ERROR("set field failed, filed_name[%s]", field.name.c_str());
						return false;
					}
				}
				cb(*pData);
			}
			row_num++;

			if (!stmt->getMoreResults())
			{
				break;
			}
		} while(true);
		return true;
	}
	catch (sql::SQLException &e) {
		L_ERROR("%s, MySQL error code:%d, SQLState:%s", e.what(), e.getErrorCode(), e.getSQLStateCStr());
		return false;
	}
}


bool MysqlCon::Query(uint16_t table_id, std::string &cond, uint32_t limit_num, QueryResultRowCb cb)
{
	L_COND_F(m_con);
	L_DEBUG("table_id=%d", table_id);
	const Table *table = TableCfg::Ins().GetTable(table_id);
	L_COND_F(table);
	unique_ptr<BaseTable> defaultData = table->factor();
	BaseTable &data = *defaultData;
	try {
		string sql_str;
		CreateSelectSql(cond, table->name, limit_num, sql_str);
		L_DEBUG("select sql [%s]", sql_str.c_str());
		unique_ptr< sql::Statement > stmt(m_con->createStatement());
		stmt->execute(sql_str);

		int row_num = 0;
		do
		{
			unique_ptr<sql::ResultSet> ret(stmt->getResultSet());
			if (0 == row_num && nullptr == ret) //一个数据都没有
			{
				L_ERROR("execute sql fail [%s]", sql_str.c_str());
				return false;
			}
			while (ret->next()) {
				unique_ptr<db::BaseTable> pData = table->factor();
				for (const Field &field : table->m_vecField)
				{
					if (!SetField(*pData, field, *ret))
					{
						L_ERROR("set field failed, filed_name[%s]", field.name.c_str());
						return false;
					}
				}
				cb(*pData);
			}
			row_num++;

			if (!stmt->getMoreResults())
			{
				break;
			}
		} while (true);
		return true;
	}
	catch (sql::SQLException &e) {
		L_ERROR("%s, MySQL error code:%d, SQLState:%s", e.what(), e.getErrorCode(), e.getSQLStateCStr());
		return false;
	}
}

bool MysqlCon::Del(const db::BaseTable &data)
{
	L_COND_F(m_con);
	const Table *table = TableCfg::Ins().GetTable(data.TableId());
	L_COND_F(table);
	try {
		string sql_str = "delete from ";
		sql_str += table->name;
		sql_str += " where ";

		{//build cond
			const Field *field = table->GetMainKey();
			L_COND_F(field);
			uint8_t *pField = (uint8_t*)&data + field->pOffset;

			sql_str += field->name;
			sql_str += "=";
			if (field->type == FieldType::t_string)
			{
				sql_str += "'" + *(string *)pField + "'";
			}
			else if(field->type == FieldType::t_uint64_t || field->type == FieldType::t_int64_t)
			{
				sql_str += StrNum::NumToStr(*(uint64_t *)pField);
			}
			else
			{
				L_ERROR("no main key");
				return false;
			}
		}

		L_DEBUG("sql=%s", sql_str.c_str());
		unique_ptr<sql::Statement> stmt(m_con->createStatement());
		stmt->execute(sql_str);
		int affect_row = stmt->getUpdateCount();
		if (affect_row == 0)
		{
			L_DEBUG("del row == 0");
		}
		return true;
	}
	catch (sql::SQLException &e) {
		L_ERROR("%s, MySQL error code:%d, SQLState:%s", e.what(), e.getErrorCode(), e.getSQLStateCStr());
		return false;
	}
}

bool MysqlCon::ExecuteSql(const std::string &sql_str)
{
	L_COND_F(m_con);
	try {
		L_DEBUG("excute sql: %s", sql_str.c_str());
		unique_ptr< sql::Statement > stmt(m_con->createStatement());
		stmt->execute(sql_str);
		return true;
	}
	catch (sql::SQLException &e) {
		L_ERROR("%s, MySQL error code:%d, SQLState:%s", e.what(), e.getErrorCode(), e.getSQLStateCStr());
		return false;
	}
}


bool MysqlCon::CreateInsertSql(const db::BaseTable &data, std::string &sql_str)
{
	const Table *table = TableCfg::Ins().GetTable(data.TableId());
	L_COND_F(table);

	sql_str += "INSERT INTO ";
	sql_str += table->name;
	sql_str += "(";

	//循环拷贝名字
	int i = 0;
	for (const Field &field : table->m_vecField)
	{
		if (i > 0)
		{
			sql_str += ",";
		}
		sql_str += field.name;
		i++;
	}

	sql_str += ") VALUES(";

	i = 0;
	for (const Field &field : table->m_vecField)
	{
		if (i > 0)
		{
			sql_str += ",?";
		}
		else
		{
			sql_str += "?";
		}
		i++;
	}

	sql_str += ");";
	return true;
}

std::string MysqlCon::GetCreateTypeStr(db::FieldType t, bool is_unique)
{
	string s;
	switch (t)
	{
	default:
		L_ERROR("unknow type %d", (int)t);
		s = "unknow type";
		break;
	case FieldType::t_double:
		s = "double";
		break;
	case FieldType::t_int64_t:
	case FieldType::t_uint64_t:
		s = "bigint(20)";
		break;
	case FieldType::t_int32_t:
	case FieldType::t_uint32_t:
		s = "int(11)";
		break;
	case FieldType::t_bool:
		s = "bool null";
		break;
	case FieldType::t_string:
		s = "varchar(255) CHARACTER SET utf8 COLLATE utf8_general_ci";
		break;
	case FieldType::t_bytes:
		s = "blob NULL";
		break;
	case FieldType::t_struct:
		s = "blob NULL";
		break;
	}
	if (is_unique)
	{
		return s;
	}
	else
	{
		return s + " NULL DEFAULT NULL";
	}
}
