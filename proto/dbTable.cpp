
#include "dbTable.h"
#include "../db_driver/src/log_def.h"
#include "dbTableDef.h"
#include <set>
#include <limits>
#include <string.h>
#include "dbStructPack.h"

using namespace std;
using namespace db;

std::unique_ptr<BaseTable> CreateT()
{
	unique_ptr<BaseTable> p;
	p.reset(new Player3());
	return p;
}

TableCfg::TableCfg()
{
	InitTableCfg();
	CheckMissField();
	CheckStructField();
}

bool db::TableCfg::Pack(const BaseTable &obj, char *cur, size_t &packLen)
{
	//打包格式：
	//uint16_t tableid，
	//uint8_t field_num	--有效域数
	//uint8_t fieldIdx[field_num]	--有效域索引 数组
	//value1, value2, ...  --有效域 的值,不同类型长度不一样。 
	//value 如果是string结构, 内存格式： {uint32_t len, char *str}
	//uint8_t endFlag; --标识结尾 tableid+field_num ，截取一个字节。 目的：更严格的检查，降低随机错误检查不出来的概率

#define  SetValue(v) db::Pack(v, cur, len)

	size_t len= packLen;
	SetValue(obj.TableId());

	auto it = m_allTable.find(obj.TableId());
	if (it == m_allTable.end())
	{
		L_ERROR("find talbeId fail, %d", obj.TableId());
		return false;
	}
	const Table &table = it->second;


	//init field_num fieldIdx
	uint8_t field_num = 0;
	static uint8_t fieldIdx[std::numeric_limits<uint8_t>::max()];
	int idx = 0;
	for (const Field &field : table.m_vecField)
	{
		uint8_t *pField = (uint8_t*)&obj + field.pOffset;
		if (field.type == FieldType::t_bytes || field.type == FieldType::t_string)
		{
			std::string *pStr = (std::string *)pField;
			if (!pStr->empty())
			{
				fieldIdx[field_num] = idx;
				field_num++;
			}
		}
		else if (field.type == FieldType::t_struct)
		{
			fieldIdx[field_num] = idx;
			field_num++;
		}
		else
		{
			static uint64_t zero = uint64_t();
			if ( 0 != memcmp(pField, &zero, field.fieldSize))
			{
				fieldIdx[field_num] = idx;
				field_num++;
			}
		}
		idx++;
	}

	SetValue(field_num);

	L_COND(len > field_num, false);
	memcpy(cur, fieldIdx, field_num);
	cur += field_num;
	len -= field_num;
	//L_DEBUG("field_num = %d", field_num);
	//init value1, value2, ...
	for (int i=0; i< field_num; ++i)
	{
		int idx = fieldIdx[i];
		const Field &field = table.m_vecField[idx];
		char *pField = (char*)&obj + field.pOffset;
		field.packFun(*pField, cur, len);
	}
	uint8_t endFlag = field_num + (uint8_t)obj.TableId();
	SetValue(endFlag);
	packLen = packLen - len;
	return true;
#undef SetValue
}

std::unique_ptr<db::BaseTable> db::TableCfg::Unpack(const char *cur, size_t len)
{
	L_COND(cur, nullptr);
#define  SetValue(v) db::Unpack(v, cur, len)


	uint16_t tableId;
	SetValue(tableId);
	uint8_t field_num;
	SetValue(field_num);

	static uint8_t fieldIdx[std::numeric_limits<uint8_t>::max()];
	for (int i = 0; i < field_num; i++)
	{
		uint8_t idx;
		SetValue(idx);
		fieldIdx[i]= idx;
	}

	auto it = m_allTable.find(tableId);
	if (it == m_allTable.end())
	{
		return nullptr;
	}
	const Table &table = it->second;
	unique_ptr<BaseTable> p = table.factor();
	for (int i = 0; i < field_num; ++i)
	{
		int idx = fieldIdx[i];
		const Field &field = table.m_vecField[idx];
		char *pField = (char*)(p.get()) + field.pOffset;

		field.unpackFun(*pField, cur, len);
	}
	uint8_t endFlag;
	SetValue(endFlag);
	if (endFlag != (field_num + (uint8_t)tableId))
	{
		return nullptr;
	}
	return p;
#undef SetValue
}

const db::Table * db::TableCfg::GetTable(uint16_t tableId) const
{
	auto it = m_allTable.find(tableId);
	if (it == m_allTable.end())
	{
		return nullptr;
	}
	return &(it->second);
}

void db::TableCfg::InitTableCfg()
{
	//init all db table decript
	std::set<uint16_t> m_tableIds; //检查id唯一
#if 0
	{ //宏实现的代码模板
		int idx = 0;
		PlayerTest t;//用来推算类名字
		Table &table = m_allTable[t.TableId()];
		table.name = t.className;
		table.tableId = t.TableId();
		L_ASSERT(m_tableIds.insert(table.tableId).second);
		L_ASSERT(0 == &(((decltype(t) *)(nullptr))->className));//检查className 必须定义第一行
		L_ASSERT(string(t.className) == "PlayerTest"); //检查类名
		size_t lastOffset = sizeof(BaseTable); //用来检查定义顺序是否和结构一致
		table.factor = []() {
			unique_ptr<BaseTable> p(new PlayerTest());
			return p;
		};
		//field 1
		{
			Field field;
			field.name = "id";
			field.type = FieldTypeTraits<decltype(t.id)>::type;
			field.pOffset = (size_t)&(((decltype(t) *)(nullptr))->id);
			field.fieldSize = sizeof(t.id);
			field.idx = table.m_vecField.size();
			if (typeid(t.id) == typeid(Bytes) || typeid(t.id) == typeid(std::string))
			{
				field.fieldSize = 0;
			}
			L_ASSERT(lastOffset <= field.pOffset); //field定义顺序和执行不一致
			lastOffset = field.pOffset;

			field.keyType = KeyType::MAIN;
			field.packFun = (PackFun)db::Pack<decltype(t.id)>;
			field.unpackFun = (UnpackFun)db::Unpack<decltype(t.id)>;
			table.m_vecField.push_back(field);
		}
		//field 2
		{
			//...
		}
		//。。。。。
		L_ASSERT(table.m_vecField.size() < std::numeric_limits<uint8_t>::max());
	}
#endif

#define DB_CLASS_NAME(def_name)\
	{\
	int idx = 0;\
	def_name t;\
	Table &table = m_allTable[t.TableId()];\
	table.name = t.className;\
	table.tableId = t.TableId();\
	L_ASSERT((m_tableIds.insert(table.tableId)).second);\
	L_ASSERT((void*)8 == &(((decltype(t) *)(nullptr))->className));\
	L_ASSERT(string(t.className) == #def_name);\
	size_t lastOffset = sizeof(BaseTable);\
	table.factor = []() {\
	unique_ptr<BaseTable> p(new def_name());\
	return p;\
	};\


#define DB_FIELD(fieldName)\
	{\
	Field field;\
	field.name = #fieldName;\
	field.type = FieldTypeTraits<decltype(t.fieldName)>::type;\
	field.pOffset = (size_t)&(((decltype(t) *)(nullptr))->fieldName);\
	field.fieldSize = sizeof(t.fieldName);\
	field.idx = table.m_vecField.size();\
	if (typeid(t.fieldName) == typeid(Bytes) || typeid(t.fieldName) == typeid(std::string))\
	{\
		field.fieldSize = 0;\
	}\
	L_ASSERT(lastOffset <= field.pOffset);\
	lastOffset = field.pOffset;\
	field.packFun = (PackFun)db::Pack<decltype(t.fieldName)>;\
	field.unpackFun = (UnpackFun)db::Unpack<decltype(t.fieldName)>;\
	table.m_vecField.push_back(field);\
	}\


#define DB_MAIN_KEY(fieldName)	 \
	{\
	Field field;\
	field.name = #fieldName;\
	field.type = FieldTypeTraits<decltype(t.fieldName)>::type;\
	field.pOffset = (size_t)&(((decltype(t) *)(nullptr))->fieldName);\
	field.fieldSize = sizeof(t.fieldName);\
	field.idx = table.m_vecField.size();\
	if (typeid(t.fieldName) == typeid(Bytes) || typeid(t.fieldName) == typeid(std::string))\
	{\
		field.fieldSize = 0;\
	}\
	L_ASSERT(lastOffset <= field.pOffset);\
	lastOffset = field.pOffset;\
	field.keyType = KeyType::MAIN;\
	field.packFun = (PackFun)db::Pack<decltype(t.fieldName)>;\
	field.unpackFun = (UnpackFun)db::Unpack<decltype(t.fieldName)>;\
	table.m_vecField.push_back(field);\
	}\


#define DB_INDEX_KEY(fieldName)	 \
	{\
	Field field;\
	field.name = #fieldName;\
	field.type = FieldTypeTraits<decltype(t.fieldName)>::type;\
	field.pOffset = (size_t)&(((decltype(t) *)(nullptr))->fieldName);\
	field.fieldSize = sizeof(t.fieldName);\
	field.idx = table.m_vecField.size();\
	if (typeid(t.fieldName) == typeid(Bytes) || typeid(t.fieldName) == typeid(std::string))\
	{\
		field.fieldSize = 0;\
	}\
	L_ASSERT(lastOffset <= field.pOffset);\
	lastOffset = field.pOffset;\
	field.keyType = KeyType::INDEX;\
	field.packFun = (PackFun)db::Pack<decltype(t.fieldName)>;\
	field.unpackFun = (UnpackFun)db::Unpack<decltype(t.fieldName)>;\
	table.m_vecField.push_back(field);\
	}\


#define DB_CLASS_END	\
	L_ASSERT(table.m_vecField.size() < std::numeric_limits<uint8_t>::max());\
}\

	DB_ALL_TABLE_INFO

#undef  DB_CLASS_NAME
#undef  DB_MAIN_KEY
#undef  DB_INDEX_KEY
#undef  DB_FIELD
#undef  DB_CLASS_END
}

void db::TableCfg::CheckMissField()
{
#if 0
	struct CheckPlayer23 : public BaseTable
	{
		using CheckType = Player2;
		CheckPlayer23() : BaseTable("CheckPlayer2", 2) {}
		decltype(CheckType::id2) id2;
	};
	L_ASSERT(sizeof(CheckPlayer23) == sizeof(Player2));
#endif
	//

#define DB_CLASS_NAME(className) \
	struct Check##className : public BaseTable\
	{\
		using CheckType = className;\
		Check##className() : BaseTable("none", 0) {}\

#define DB_MAIN_KEY(fieldName)\
		decltype(CheckType::fieldName) fieldName;\

#define DB_INDEX_KEY(fieldName)\
		decltype(CheckType::fieldName) fieldName;\

#define DB_FIELD(fieldName)	\
		decltype(CheckType::fieldName) fieldName;\

#define DB_CLASS_END\
	};\

	DB_ALL_TABLE_INFO

#undef  DB_CLASS_NAME
#undef  DB_MAIN_KEY
#undef  DB_INDEX_KEY
#undef  DB_FIELD
#undef  DB_CLASS_END

#define DB_CLASS_NAME(className) \
	L_ASSERT(sizeof(Check##className) == sizeof(className));\

#define DB_MAIN_KEY(fieldName)
#define DB_INDEX_KEY(fieldName)
#define DB_FIELD(fieldName)	
#define DB_CLASS_END

		DB_ALL_TABLE_INFO

#undef  DB_CLASS_NAME
#undef  DB_MAIN_KEY
#undef  DB_INDEX_KEY
#undef  DB_FIELD
#undef  DB_CLASS_END
}

void db::TableCfg::CheckStructField()
{
#if 0
	struct Check_Ride
	{
		using CheckType = Ride;
		decltype(CheckType::id) id;
		decltype(CheckType::ids) ids;
	};
	L_ASSERT(sizeof(Check_Ride) == sizeof(Check_Ride::CheckType)); //确保域宏定义完整

		//宏实现的代码模板
	{
		Ride t;
		size_t lastOffset = 0; //用来检查定义顺序是否和结构一致
		//field 1
		{
			size_t offset = (size_t)&(((decltype(t) *)(nullptr))->id);
			L_ASSERT(lastOffset <= offset); //field定义顺序和执行不一致
			lastOffset = offset;
		}
		//field 2
		//。。。。。
	}
#endif
	//

#define DB_CLASS_NAME(className) \
	struct Check_##className \
	{\
		using CheckType = className;\

#define DB_FIELD(fieldName)	\
		decltype(CheckType::fieldName) fieldName;\

#define DB_CLASS_END\
	};\

	DB_ALL_STRUCT_INFO

#undef  DB_CLASS_NAME
#undef  DB_FIELD
#undef  DB_CLASS_END

#define DB_CLASS_NAME(className) \
	L_ASSERT(sizeof(Check_##className) == sizeof(Check_##className::CheckType));\

#define DB_FIELD(fieldName)	
#define DB_CLASS_END

		DB_ALL_STRUCT_INFO

#undef  DB_CLASS_NAME
#undef  DB_FIELD
#undef  DB_CLASS_END


#define DB_CLASS_NAME(className) \
	{\
		className t;\
		size_t lastOffset = 0; \

#define DB_FIELD(fieldName)	\
	{\
		size_t offset = (size_t)&(((decltype(t) *)(nullptr))->fieldName);\
		L_ASSERT(lastOffset <= offset); \
		lastOffset = offset;\
	}\

#define DB_CLASS_END\
	}\

		DB_ALL_STRUCT_INFO

#undef  DB_CLASS_NAME
#undef  DB_FIELD
#undef  DB_CLASS_END

}

const db::Field * db::Table::GetMainKey() const
{
	for (const Field &field : m_vecField)
	{
		if (KeyType::MAIN == field.keyType)
		{
			return &field;
		}
	}
	return nullptr;
}
