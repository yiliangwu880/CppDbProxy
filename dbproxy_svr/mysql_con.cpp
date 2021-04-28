#include "db_con.h"
#include <cppconn/driver.h>
#include <cppconn/datatype.h>
#include <mysql_driver.h>
#include <cppconn/prepared_statement.h>
#include "parser.h"
#include "mysql_con.h"
#include "svr_util/include/string_tool.h"
#include "DbServer.h"

using namespace su;
using namespace lc;
using namespace std;
using namespace google::protobuf;
using namespace db;
using UINT64 = ::uint64;


void MysqlCon::CreateSelectSql(const db::ReqGetData &req, const string &table_name, string &sql_str)
{
	sql_str = "SELECT ";
	sql_str += "* FROM ";
	sql_str += table_name;
	if (!req.cond().empty())
	{
		sql_str += " WHERE " + req.cond();
	}
	if (req.limit_num() != 0)
	{
		sql_str += " limit ";
		sql_str += StringTool::NumToStr(req.limit_num());
	}
}

bool MysqlCon::InitTable(const ReqInitTable &req, RspInitTable &rsp)
{
	rsp.Clear();
	L_COND_F(m_con);
	for (const string &msg_name : req.msg_name())
	{
		try {
			string sql_str;
			L_DEBUG("TryCreateTableSql msg_name=%s", msg_name.c_str());
			if (!TryCreateTableSql(msg_name, sql_str))
			{
				L_ERROR("CreateSql fail");
				return false;
			}

			L_DEBUG("create sql=%s", sql_str.c_str());
			std::unique_ptr<sql::PreparedStatement> pstmt(m_con->prepareStatement(sql_str));
			int affect_row = pstmt->executeUpdate();
			if (0 != affect_row)
			{
				L_ERROR("create table fail. affect_row=%d, table name[%s], sql[%s]", affect_row, msg_name.c_str(), sql_str.c_str());
				return false;
			}
		}
		catch (sql::SQLException &e) {
			L_ERROR("%s, MySQL error code:%d, SQLState:%s", e.what(), e.getErrorCode(), e.getSQLStateCStr());
			return false;
		}
	}
	rsp.set_is_ok(true);
	return true;
}

bool MysqlCon::Insert(const db::ReqInsertData &req, ::uint64 &num_key, std::string &str_key)
{
	L_COND_F(m_con);
	unique_ptr<google::protobuf::Message> msg = ProtoUtil::CreateMessage(req.msg_name());
	if (nullptr == msg)
	{
		L_ERROR("create message fail. name=%s", req.msg_name().c_str());
		return false;
	}
	L_COND_F(msg->ParseFromString(req.data()), "parse msg fail. msg_name = %s", req.msg_name().c_str());

	if (!ProtoUtil::GetMsgMainKeyVal(*msg, num_key, str_key))
	{
		L_WARN("illegal message %s. no main key. ", req.msg_name().c_str());
		return false;
	}

	//LOG_DEBUG("msg content=%s", msg->DebugString().c_str());
	try {
		string sql_str;
		CreateInsertSql(*msg, sql_str);

		//LOG_DEBUG("create insert sql=%s", sql_str.c_str());
		unique_ptr< sql::PreparedStatement > pstmt(m_con->prepareStatement(sql_str));
		SetInsertPreparePara(*pstmt, *msg);
		int affect_row = pstmt->executeUpdate();
		if (1 != affect_row)
		{
			L_ERROR("insert table fail. row=%d, table name[%s]", affect_row, req.msg_name().c_str());
			return false;
		}
		return true;
	}
	catch (sql::SQLException &e) {
		L_ERROR("%s, MySQL error code:%d, SQLState:%s", e.what(), e.getErrorCode(), e.getSQLStateCStr());
		return false;
	}
}

bool MysqlCon::Update(const db::ReqUpdateData &req, ::uint64 &num_key, std::string &str_key)
{
	L_COND_F(m_con);
	unique_ptr<google::protobuf::Message> msg = ProtoUtil::CreateMessage(req.msg_name());
	if (nullptr == msg)
	{
		L_ERROR("create message fail. name=%s", req.msg_name().c_str());
		return false;
	}
	L_COND_F(msg->ParseFromString(req.data()), "parse msg fail. msg_name = %s", req.msg_name().c_str());
	if (!ProtoUtil::GetMsgMainKeyVal(*msg, num_key, str_key))
	{
		L_WARN("illegal message %s. no main key. ", req.msg_name().c_str());
		return false;
	}

	try {
		string sql_str;
		if (!CreateUpdateSql(*msg, sql_str))
		{
			L_ERROR("create update sql fail");
			return false;
		}
		L_DEBUG("create update sql=%s", sql_str.c_str());
		unique_ptr< sql::PreparedStatement > pstmt(m_con->prepareStatement(sql_str));
		SetUpdatePreparePara(*pstmt, *msg);
		int affect_row = pstmt->executeUpdate();
		if (1 != affect_row)
		{
			L_ERROR("update table fail. row=%d, table name[%s]", affect_row, req.msg_name().c_str());
			return false;
		}
		return true;
	}
	catch (sql::SQLException &e) {
		L_ERROR("%s, MySQL error code:%d, SQLState:%s", e.what(), e.getErrorCode(), e.getSQLStateCStr());
		return false;
	}
}

namespace
{
	class DataBuf : public std::streambuf
	{
	public:
		DataBuf() {}

		DataBuf(char * d, size_t s) {
			setg(d, d, d + s);
		}

		void InitBuf(char * d, size_t s) {
			setg(d, d, d + s);
		}
	};

	int GetType(FieldDescriptor::Type fd_type)
	{
		switch (fd_type)
		{
		default:
			L_ERROR("unknow type %d", (int)fd_type);
			return sql::DataType::INTEGER;
		case FieldDescriptor::TYPE_MESSAGE:
			return sql::DataType::BINARY;
			break;
		case FieldDescriptor::TYPE_DOUBLE:
			return sql::DataType::DOUBLE;
			break;
		case FieldDescriptor::TYPE_FLOAT:
			return sql::DataType::DOUBLE;
			break;
		case FieldDescriptor::TYPE_INT64:
		case FieldDescriptor::TYPE_SINT64:
		case FieldDescriptor::TYPE_SFIXED64:
			return sql::DataType::BIGINT;
			break;
		case FieldDescriptor::TYPE_UINT64:
		case FieldDescriptor::TYPE_FIXED64:
			return sql::DataType::BIGINT;
			break;
		case FieldDescriptor::TYPE_FIXED32:
		case FieldDescriptor::TYPE_UINT32:
			return sql::DataType::INTEGER;
			break;
		case FieldDescriptor::TYPE_INT32:
		case FieldDescriptor::TYPE_SFIXED32:
		case FieldDescriptor::TYPE_SINT32:
			return sql::DataType::INTEGER;
			break;
		case FieldDescriptor::TYPE_ENUM:
			return sql::DataType::INTEGER;
			break;
		case FieldDescriptor::TYPE_BOOL:
			return sql::DataType::TINYINT;
			break;
		case FieldDescriptor::TYPE_STRING:
			return sql::DataType::VARCHAR;
			break;
		case FieldDescriptor::TYPE_BYTES:
			return sql::DataType::BINARY;
			break;
		}
	}

	void SetFieldParam(google::protobuf::Message &msg, const FieldDescriptor &field, sql::PreparedStatement &pstmt, int idx)
	{
		const Reflection* rfl = msg.GetReflection();
		L_COND(rfl);
		if (field.is_repeated())
		{
			L_ERROR("not support repeated lable");
			return;
		}
		if (!rfl->HasField(msg, &field))
		{
			pstmt.setNull(idx, GetType(field.type()));
			return;
		}

		switch (field.type())
		{
		default:
			L_ERROR("unknow type %d", (int)field.type());
			break;
		case FieldDescriptor::TYPE_DOUBLE:
			pstmt.setDouble(idx, rfl->GetDouble(msg, &field));
			break;
		case FieldDescriptor::TYPE_FLOAT:
			pstmt.setDouble(idx, rfl->GetFloat(msg, &field));
			break;
		case FieldDescriptor::TYPE_INT64:
		case FieldDescriptor::TYPE_SINT64:
		case FieldDescriptor::TYPE_SFIXED64:
			pstmt.setInt64(idx, rfl->GetInt64(msg, &field));
			break;
		case FieldDescriptor::TYPE_UINT64:
		case FieldDescriptor::TYPE_FIXED64:
			pstmt.setUInt64(idx, rfl->GetUInt64(msg, &field));
			break;
		case FieldDescriptor::TYPE_FIXED32:
		case FieldDescriptor::TYPE_UINT32:
			pstmt.setUInt(idx, rfl->GetUInt32(msg, &field));
			break;
		case FieldDescriptor::TYPE_INT32:
		case FieldDescriptor::TYPE_SFIXED32:
		case FieldDescriptor::TYPE_SINT32:
			pstmt.setInt(idx, rfl->GetInt32(msg, &field));
			break;
		case FieldDescriptor::TYPE_ENUM:
		{
			const EnumValueDescriptor* ev = rfl->GetEnum(msg, &field);
			if (nullptr == ev)
			{
				L_ERROR("nullptr == ev");
				break;
			}
			pstmt.setInt(idx, ev->number());
			break;
		}
		case FieldDescriptor::TYPE_BOOL:
			pstmt.setBoolean(idx, rfl->GetBool(msg, &field));
			break;
		case FieldDescriptor::TYPE_STRING:
		case FieldDescriptor::TYPE_BYTES:
			pstmt.setString(idx, rfl->GetString(msg, &field));
			break;
		case FieldDescriptor::TYPE_MESSAGE:
		{
			Message* chiledMsg = rfl->MutableMessage(&msg, &field);
			L_COND(chiledMsg);
			string s = chiledMsg->SerializeAsString();
			pstmt.setString(idx, s);

			//别删，如果字符处理不了的字符，就用下面的。
			//{
			//	char str[MAX_SQL_STR_SIZE];
			//	bool r = chiledMsg->SerializeToArray(str, sizeof(str));
			//	if (!r)
			//	{
			//		L_ERROR("SerializeToArray fail");
			//		break;
			//	}

			//	DataBuf buffer;
			//	std::istream blobStream(&buffer);
			//	buffer.InitBuf(str, chiledMsg->ByteSize());
			//	blobStream.rdbuf(&buffer);
			//	pstmt.setBlob(idx, &blobStream);
			//}
			break;
		}
		} //end switch (field.type())
	}

}

void MysqlCon::SetInsertPreparePara(sql::PreparedStatement &pstmt, google::protobuf::Message &msg)
{
	const Descriptor* des = msg.GetDescriptor();
	L_COND(des);

	int cnt = des->field_count();
	L_COND(cnt >= 1);

	for (int i = 0; i < cnt; i++)
	{
		const FieldDescriptor* field = des->field(i);
		SetFieldParam(msg, *field, pstmt, i + 1);
	}
	return;
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
		
		return true;
	}
	catch (sql::SQLException &e) {
		L_ERROR("%s, MySQL error code:%d, SQLState:%s", e.what(), e.getErrorCode(), e.getSQLStateCStr());
		return false;
	}
}

bool MysqlCon::TryCreateTableSql(const std::string &msg_name, std::string &sql_str)
{
	const Descriptor* des = DescriptorPool::generated_pool()->FindMessageTypeByName(msg_name);
	if (nullptr == des)
	{
		L_ERROR("find msg Descriptor fail. msg name=%s", msg_name.c_str());
		return false;
	}
	if (0 == des->field_count())
	{
		L_ERROR("msg no field. msg name=%s", msg_name.c_str());
		return false;
	}


	sql_str = "CREATE TABLE IF NOT EXISTS `";
	sql_str += des->name(); //不用 msg_name 去掉命名空间, 因为表名有.符号会导致insert sql语句查表名失败
	sql_str += "` (";
	
	int cnt = des->field_count();
	string main_key;
	VecStr index_key;
	for (int i = 0; i < cnt; i++)
	{
		//每个域 字符串模板 = "`a` varchar(11) NOT NULL,"
		const FieldDescriptor* field = des->field(i);
		if (nullptr == field)
		{
			L_ERROR("unknow error %d", i);
			return false;
		}
		if (field->is_repeated())
		{
			L_ERROR("not support repeated field");
			return false;
		}
		
		sql_str += "`";
		sql_str += field->name();
		sql_str += "` ";

		KeyType key_type;
		bool is_unique = false; 
		{
			ProtoUtil::GetFieldKeyOpt(*des, field->name(), key_type);
			if (db::K_MAIN_KEY == key_type)
			{
				main_key = field->name();
				is_unique = true;
			}
			else if (db::K_INDEX == key_type)
			{
				index_key.push_back(field->name());
				is_unique = true;
			}
		}
		sql_str += GetCreateTypeStr(field->type(), is_unique);
		sql_str += ",";
		
	}
	if (main_key.empty())
	{
		L_ERROR("msg main key need set field opt [(OptKey)=K_MAIN_KEY]. msg name=%s", msg_name.c_str());
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

bool MysqlCon::SetField(Message& msg, const FieldDescriptor &field, const sql::ResultSet& res)
{
	const Reflection* rfl = msg.GetReflection();
	L_COND_F(rfl);

	if (field.is_repeated())
	{
		L_ERROR("not support repeated field");
		return false;
	}
	if (res.isNull(field.name()))
	{
		return true;
	}
	try {
		switch (field.type())
		{
		default:
			L_ERROR("unknow type %d", (int)field.type());
			return false;
		case FieldDescriptor::TYPE_DOUBLE:
			rfl->SetDouble(&msg, &field, (double)res.getDouble(field.name()));
			break;
		case FieldDescriptor::TYPE_FLOAT:
			rfl->SetFloat(&msg, &field, (float)res.getDouble(field.name()));
			break;
		case FieldDescriptor::TYPE_INT64:
		case FieldDescriptor::TYPE_SINT64:
		case FieldDescriptor::TYPE_SFIXED64:
			rfl->SetInt64(&msg, &field, res.getInt64(field.name()));
			break;
		case FieldDescriptor::TYPE_UINT64:
		case FieldDescriptor::TYPE_FIXED64:
			rfl->SetUInt64(&msg, &field, res.getUInt64(field.name()));
			break;
		case FieldDescriptor::TYPE_FIXED32:
		case FieldDescriptor::TYPE_UINT32:
			rfl->SetUInt32(&msg, &field, res.getUInt(field.name()));
			break;
		case FieldDescriptor::TYPE_INT32:
		case FieldDescriptor::TYPE_SFIXED32:
		case FieldDescriptor::TYPE_SINT32:
			rfl->SetInt32(&msg, &field, res.getInt(field.name()));
			break;
		case FieldDescriptor::TYPE_ENUM:
		{
			const EnumDescriptor *pEnumDes = field.enum_type();
			const EnumValueDescriptor* pV = pEnumDes->FindValueByNumber(res.getInt(field.name()));
			if (nullptr == pV)
			{
				L_ERROR("can't find Enum value. value=%d", res.getInt(field.name()));
				break;
			}
			rfl->SetEnum(&msg, &field, pV);
		}
		break;
		case FieldDescriptor::TYPE_BOOL:
			rfl->SetBool(&msg, &field, res.getBoolean(field.name()));
			break;
		case FieldDescriptor::TYPE_STRING:
			rfl->SetString(&msg, &field, res.getString(field.name()));
			break;
		case FieldDescriptor::TYPE_BYTES:
			rfl->SetString(&msg, &field, res.getString(field.name()));
			break;
		case FieldDescriptor::TYPE_MESSAGE:
		{
			unique_ptr<std::istream> inStream(res.getBlob(field.name()));
			if (nullptr == inStream)
			{
				L_ERROR("res.getBlob() fail field.name()=%s", field.name().c_str());
				return false;
			}
			//NULL不解析消息
			if (res.wasNull())
			{
				return true;
			}
			Message* chiledMsg = rfl->MutableMessage(&msg, &field);
			if (nullptr == chiledMsg)
			{
				L_ERROR("MutableMessage fail field.name()=%s", field.name().c_str());
				return false;
			}

			if (!chiledMsg->ParseFromIstream(inStream.get()))
			{
				L_ERROR("解析失败！");
				return false;
			}
			break;
		}
		}//end switch (field.type())
	}
	catch (sql::InvalidArgumentException e) {
		//结果集中没有改字段，不进行设置
		L_ERROR("can not set filed %s, ", field.name().c_str());
		L_ERROR("%s, MySQL error code:%d, SQLState:%d", e.what(), e.getErrorCode(), e.getSQLStateCStr());
		return false;
	}
	catch (sql::SQLException &e) {
		L_ERROR("%s, MySQL error code:%d, SQLState:%s", e.what(), e.getErrorCode(), e.getSQLStateCStr());
		return false;
	}
	return true;
}

void MysqlCon::SetUpdatePreparePara(sql::PreparedStatement &pstmt, google::protobuf::Message &msg)
{
	const Descriptor* des = msg.GetDescriptor();
	L_COND(des);
	const Reflection* rfl = msg.GetReflection();
	L_COND(rfl);
	int cnt = des->field_count();
	L_COND(cnt>=1);

	int idx = 1;
	for (int i = 0; i < cnt; i++)
	{
		const FieldDescriptor* field = des->field(i);
		if (!rfl->HasField(msg, field))
		{
			continue;
		}

		KeyType key_type;
		ProtoUtil::GetFieldKeyOpt(*des, field->name(), key_type);
		if (db::K_MAIN_KEY == key_type)
		{
			continue;
		}
		SetFieldParam(msg, *field, pstmt, idx);
		idx++;
	}

	return;
}

bool MysqlCon::CreateUpdateSql(const google::protobuf::Message &msg, std::string &sql_str)
{
	const Descriptor* des = msg.GetDescriptor();
	const Reflection* ref = msg.GetReflection();
	int cnt = des->field_count();
	L_COND_F(des);
	L_COND_F(ref);
	L_COND_F(cnt >= 1);

	sql_str += "UPDATE ";
	sql_str += des->name();
	sql_str += " SET ";

	bool need_comma = false;
	string str_key;
	UINT64 num_key = 0; 
	const FieldDescriptor* key_field = nullptr;
	for (int i = 0; i < cnt; i++)
	{
		const FieldDescriptor* field = des->field(i);
		if (!ref->HasField(msg, field))
		{
			continue;
		}
		KeyType key_type;
		ProtoUtil::GetFieldKeyOpt(*des, field->name(), key_type);
		if (db::K_MAIN_KEY == key_type)
		{
			if (FieldDescriptor::TYPE_STRING == field->type())
			{
				str_key = ref->GetString(msg, field);
			}
			else//其他一致认为是uint64处理
			{
				num_key = ref->GetUInt64(msg, field);
			}
			key_field = field;
			continue;
		}
		if (need_comma)
		{
			sql_str += ",";
		}
		sql_str += field->name();
		sql_str += "=?";
		need_comma = true;
	}
	L_COND_F(key_field);
	sql_str += " ";
	char where[1000]; 
	if (0 == num_key)
	{
		str_key = ref->GetString(msg, key_field);
		snprintf(where, sizeof(where), "WHERE %s='%s'  limit 1", key_field->name().c_str(), str_key.c_str());
	}
	else//其他一致认为是uint64处理
	{
		num_key = ref->GetUInt64(msg, key_field);
		snprintf(where, sizeof(where), "WHERE %s=%llu  limit 1", key_field->name().c_str(), num_key);
	}
	sql_str += where;

	sql_str += ";";
	return true;
}

bool MysqlCon::Get(const db::ReqGetData &req, InnerSvrCon &con)
{
	L_COND_F(m_con); 
	unique_ptr<google::protobuf::Message> msg = ProtoUtil::CreateMessage(req.msg_name());
	if (nullptr == msg)
	{
		L_ERROR("create message fail. name=%s", req.msg_name().c_str());
		return false;
	}
	const google::protobuf::Descriptor* des = msg->GetDescriptor();
	L_COND_F(des);

	try {

		string sql_str;
		CreateSelectSql(req, des->name(), sql_str);
//		L_DEBUG("select sql [%s]", sql_str.c_str());
		unique_ptr< sql::Statement > stmt(m_con->createStatement());
		stmt->execute(sql_str);

		int row_num = 0;
		do
		{
			db::RspGetData rsp;
			rsp.set_msg_name(req.msg_name());

			unique_ptr<sql::ResultSet> ret(stmt->getResultSet());
			if (0 == row_num && nullptr == ret) //一个数据都没有
			{
				L_ERROR("execute sql fail [%s]", sql_str.c_str());
				return false;
			}
			while (ret->next()) {
				msg->Clear();
				int count = des->field_count();
				for (int i = 0; i < count; i++)
				{
					const FieldDescriptor* field = des->field(i);
					if (!SetField(*msg, *field, *ret))
					{
						L_ERROR("set field failed, filed_name[%s]", field->name().c_str());
						return false;
					}
				}
				rsp.add_data(msg->SerializeAsString());
			}
			row_num++;

			if (!stmt->getMoreResults())
			{
				rsp.set_is_last(true);
				con.Send(rsp);
				break;
			}
			con.Send(rsp);
		} while(true);
		return true;
	}
	catch (sql::SQLException &e) {
		L_ERROR("%s, MySQL error code:%d, SQLState:%s", e.what(), e.getErrorCode(), e.getSQLStateCStr());
		return false;
	}
}

bool MysqlCon::Del(const db::ReqDelData &req, db::RspDelData &rsp)
{
	rsp.Clear();
	rsp.set_msg_name(req.msg_name());
	rsp.set_num_key(req.num_key());
	rsp.set_str_key(req.str_key());
	L_COND_F(m_con);

	const Descriptor* des = DescriptorPool::generated_pool()->FindMessageTypeByName(req.msg_name());
	if (nullptr == des)
	{
		L_WARN("illegal msg name. %s", req.msg_name().c_str());
		return false;
	}
	try {
		string sql_str = "delete from ";
		sql_str += des->name();
		sql_str += " where ";

		{//build cond
			string key_name = ProtoUtil::GetMsgMainKeyName(*des);
			if (key_name.empty())
			{
				L_WARN("illegal message %s. no main key. ", req.msg_name().c_str());
				return false;
			}

			sql_str += key_name;
			sql_str += "=";
			if (req.num_key() != 0)
			{
				sql_str += StringTool::NumToStr(req.num_key());
			}
			else
			{
				sql_str += "'"+req.str_key()+"'";
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
		rsp.set_del_num(affect_row);
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
		unique_ptr< sql::Statement > stmt(m_con->createStatement());
		stmt->execute(sql_str);
		return true;
	}
	catch (sql::SQLException &e) {
		L_ERROR("%s, MySQL error code:%d, SQLState:%s", e.what(), e.getErrorCode(), e.getSQLStateCStr());
		return false;
	}
}

bool MysqlCon::CreateInsertSql(const google::protobuf::Message &msg, std::string &sql_str)
{
	const Descriptor* des = msg.GetDescriptor();
	L_COND_F(des);
	int count = des->field_count();
	L_COND_F(count>0);

	sql_str += "INSERT INTO ";
	sql_str += des->name();
	sql_str += "(";

	//循环拷贝名字
	for (int i = 0; i < count; i++)
	{
		const FieldDescriptor* field = des->field(i);
		if (i > 0)
		{
			sql_str += ",";
		}
		sql_str += field->name();
	}

	sql_str += ") VALUES(";

	for (int i = 0; i < count; i++)
	{
		if (i > 0)
		{
			sql_str += ",?";
		}
		else
		{
			sql_str += "?";
		}
	}

	sql_str += ");";
	return true;
}

std::string MysqlCon::GetCreateTypeStr(FieldDescriptor::Type t, bool is_unique)
{
	string s;
	switch (t)
	{
	default:
		L_ERROR("unknow type %d", (int)t);
		s="unknow type";
		break;
	case FieldDescriptor::TYPE_DOUBLE:
	case FieldDescriptor::TYPE_FLOAT:
		s = "double";
		break;
	case FieldDescriptor::TYPE_INT64:
	case FieldDescriptor::TYPE_SINT64:
	case FieldDescriptor::TYPE_SFIXED64:
	case FieldDescriptor::TYPE_UINT64:
	case FieldDescriptor::TYPE_FIXED64:
	case FieldDescriptor::TYPE_FIXED32:
	case FieldDescriptor::TYPE_UINT32:
		s = "bigint(20)";
		break;
	case FieldDescriptor::TYPE_INT32:
	case FieldDescriptor::TYPE_ENUM:
	case FieldDescriptor::TYPE_SFIXED32:
	case FieldDescriptor::TYPE_SINT32:
	case FieldDescriptor::TYPE_BOOL:
		s = "int(11)";
		break;
	case FieldDescriptor::TYPE_STRING:
		s = "varchar(255) CHARACTER SET utf8 COLLATE utf8_general_ci";
		break;
	case FieldDescriptor::TYPE_MESSAGE:
	case FieldDescriptor::TYPE_BYTES:
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