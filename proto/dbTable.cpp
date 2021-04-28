
#include "dbTable.h"
#include "../db_driver/src/log_def.h"
#include "dbTableDef.h"
#include <set>
#include <limits>
#include <string.h>

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
	//init all db table decript
	std::set<uint16_t> m_tableIds; //检查id唯一
#if 0
	{ //宏实现的代码模板
		int idx = 0;
		PlayerTest t;//用来推算类名字
		Table &table = m_allTable[t.TableId()];
		table.name = t.className;
		table.tableId = t.TableId();
		L_ASSERT(m_tableIds.insert(table.tableId).first);
		L_ASSERT(0 == &(((decltype(t) *)(nullptr))->className));//检查className 必须定义第一行
		L_ASSERT(string(t.className) == "PlayerTest"); //检查类名
		size_t lastOffset = sizeof(BaseTable); //用来检查定义顺序是否和结构一致
		table.factor = []() {
			unique_ptr<BaseTable> p;
			p.reset(new PlayerTest());
			return p;
		};
		//field 1
		{
			Field &field = table.m_allField["id"];
			field.name = "id";
			field.type = FieldTypeTraits<decltype(t.id)>::type;
			field.pOffset = (size_t)&(((decltype(t) *)(nullptr))->id);
			field.fieldSize = sizeof(t.id);
			if (typeid(t.id)==typeid(Bytes) || typeid(t.id) == typeid(std::string))
			{
				field.fieldSize = 0;
			}
			L_ASSERT(lastOffset <= field.pOffset); //field定义顺序和执行不一致
			lastOffset = field.pOffset;

			table.pKeyOffset = (uint8_t *)&(((decltype(t) *)(nullptr))->id);
			field.keyType = KeyType::MAIN;
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
	L_ASSERT(0 == &(((decltype(t) *)(nullptr))->className));\
	L_ASSERT(string(t.className) == #def_name);\
	size_t lastOffset = sizeof(BaseTable);\
	table.factor = []() {\
	unique_ptr<BaseTable> p;\
	p.reset(new def_name());\
	return p;\
	};\


#define DB_FIELD(fieldName)\
	{\
	Field &field = table.m_allField[#fieldName];\
	field.name = #fieldName;\
	field.type = FieldTypeTraits<decltype(t.fieldName)>::type;\
	field.pOffset = (size_t)&(((decltype(t) *)(nullptr))->fieldName);\
	field.fieldSize = sizeof(t.fieldName);\
	if (typeid(t.fieldName) == typeid(Bytes) || typeid(t.fieldName) == typeid(std::string))\
	{\
		field.fieldSize = 0;\
	}\
	L_ASSERT(lastOffset <= field.pOffset);\
	lastOffset = field.pOffset;\
	table.m_vecField.push_back(field);\
	}\


#define DB_MAIN_KEY(fieldName)	 \
	{\
	Field &field = table.m_allField[#fieldName];\
	field.name = #fieldName;\
	field.type = FieldTypeTraits<decltype(t.fieldName)>::type;\
	field.pOffset = (size_t)&(((decltype(t) *)(nullptr))->fieldName);\
	field.fieldSize = sizeof(t.fieldName);\
	if (typeid(t.fieldName) == typeid(Bytes) || typeid(t.fieldName) == typeid(std::string))\
	{\
		field.fieldSize = 0;\
	}\
	L_ASSERT(lastOffset <= field.pOffset);\
	lastOffset = field.pOffset;\
	table.pKeyOffset = (size_t)&(((decltype(t) *)(nullptr))->fieldName);\
	field.keyType = KeyType::MAIN;\
	table.m_vecField.push_back(field);\
	}\


#define DB_INDEX_KEY(fieldName)	 \
	{\
	Field &field = table.m_allField[#fieldName];\
	field.name = #fieldName;\
	field.type = FieldTypeTraits<decltype(t.fieldName)>::type;\
	field.pOffset = (size_t)&(((decltype(t) *)(nullptr))->fieldName);\
	field.fieldSize = sizeof(t.fieldName);\
	if (typeid(t.fieldName) == typeid(Bytes) || typeid(t.fieldName) == typeid(std::string))\
	{\
		field.fieldSize = 0;\
	}\
	L_ASSERT(lastOffset <= field.pOffset);\
	lastOffset = field.pOffset;\
	field.keyType = KeyType::INDEX;\
	table.m_vecField.push_back(field);\
	}\


#define DB_CLASS_END	\
	L_ASSERT(table.m_vecField.size() < std::numeric_limits<uint8_t>::max());\
}\

		DB_ALL_TABLE_INFO
#undef  DB_CLASS_NAME
#undef  DB_FIELD
#undef  DB_CLASS_END
}

bool db::TableCfg::GetFieldPoint(const BaseTable &obj, const std::string &fieldName, FieldInfo &fieldInfo)
{
	auto it = m_allTable.find(obj.TableId());
	if (it == m_allTable.end())
	{
		L_ERROR("obj is not db table");
		return false;
	}
	Table &table = it->second;

	auto it_field = table.m_allField.find(fieldName);
	if (it_field == table.m_allField.end())
	{
		L_ERROR("fieldName is error %s", fieldName.c_str());
		return false;
	}
	Field &field = it_field->second;

	fieldInfo.fieldPoint = (void *)(((uint8_t *)(&obj) + (int64_t)field.pOffset));
	fieldInfo.type = field.type;
	return true;
}

bool db::TableCfg::Pack(const BaseTable &obj, char *str, size_t &len)
{
	//打包格式：
	//uint16_t tableid，
	//uint8_t field_num	--有效域数
	//uint8_t fieldIdx[field_num]	--有效域索引 数组
	//value1, value2, ...  --有效域 的值,不同类型长度不一样。 
	//value 如果是string结构, 内存格式： {uint32_t len, char *str}

#define  SetValue(v)\
	L_COND(remainLen > sizeof(v), false);\
	*(decltype(v)*)cur = v;\
	cur += sizeof(v);\
	remainLen -= sizeof(v);\

	char *cur = str; //当前些指针
	size_t remainLen = len; //剩余空间

	SetValue(obj.TableId());

	auto it = m_allTable.find(obj.TableId());
	if (it == m_allTable.end())
	{
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
	L_COND(remainLen > field_num, false);
	memcpy(cur, fieldIdx, field_num);
	cur += field_num;
	remainLen -= field_num;

	//init value1, value2, ...
	for (int i=0; i< field_num; ++i)
	{
		int idx = fieldIdx[i];
		const Field &field = table.m_vecField[idx];
		uint8_t *pField = (uint8_t*)&obj + field.pOffset;
		if (field.type == FieldType::t_bytes || field.type == FieldType::t_string)
		{
			std::string *pStr = (std::string *)pField;
			SetValue(pStr->length());

			L_COND(remainLen > pStr->length(), false);
			memcpy(cur, pStr->c_str(), pStr->length());
			cur += pStr->length();
			remainLen -= pStr->length();
		}
		else
		{
			L_COND(remainLen > field.fieldSize, false);
			memcpy(cur, pField, field.fieldSize);
			cur += field.fieldSize;
			remainLen -= field.fieldSize;
		}
	}
	len = len - remainLen;
	return true;
#undef SetValue
}

std::unique_ptr<db::BaseTable> db::TableCfg::Unpack(const char *cur, size_t len)
{
#define  SetValue(v)\
	L_COND(remainLen > sizeof(v), nullptr);\
	v = *(const decltype(v)*)cur;\
	cur += sizeof(v);\
	remainLen -= sizeof(v);\

	L_COND(cur, nullptr);
	size_t remainLen = len; //剩余空间


	uint16_t tableId;
	SetValue(tableId);
	uint8_t field_num;
	SetValue(field_num);
	if (0 == field_num)
	{
		return nullptr;
	}

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
		uint8_t *pField = (uint8_t*)(p.get()) + field.pOffset;
		if (field.type == FieldType::t_bytes || field.type == FieldType::t_string)
		{
			std::string *pStr = (std::string *)pField;
			size_t len;
			SetValue(len);
			L_COND(len > 0, nullptr);

			L_COND(remainLen >= len, nullptr);
			pStr->assign(cur, len);
			cur += len;
			remainLen -= len;
		}
		else
		{
			L_COND(remainLen >= field.fieldSize, nullptr);
			memcpy(pField, cur, field.fieldSize);
			cur += field.fieldSize;
			remainLen -= field.fieldSize;
		}
	}

	return p;
#undef SetValue
}

