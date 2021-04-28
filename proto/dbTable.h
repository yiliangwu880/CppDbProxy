/*
"对象-关系映射"（Object/Relational Mapping）
*/
#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <functional>

namespace db {
	//定义类目的，为了让 TableCfg::GetFieldPoint 能编译期 检查错误调用
	class BaseTable
	{
	public:
		friend class TableCfg;
		BaseTable(const char *name, uint16_t id) : className(name), tableId(id){}

		uint16_t TableId() const { return tableId; }
	private:
		const char *className = nullptr;
		uint16_t tableId; //table类型唯一标识
	};
	enum class KeyType
	{
		NONE,
		MAIN,
		INDEX,
	};
	enum class FieldType
	{
		t_bool,
		t_int8_t  ,
		t_uint8_t,
		t_int16_t,
		t_uint16_t,
		t_int32_t,
		t_uint32_t,
		t_int64_t,
		t_uint64_t,
		t_double,
		t_float ,
		t_string,
		t_bytes ,
	};
	//用户接口当string用，这里定义为了区分db 的 string 和 bytes
	struct Bytes : public  std::string
	{
	};

	//根据 真实类型 获取 FieldType::xx
	template<typename T>
	struct FieldTypeTraits
	{
		static const FieldType type = FieldType::t_int8_t;
	};

	template<>struct FieldTypeTraits<std::string> { static const FieldType type = FieldType::t_string; };
	template<>struct FieldTypeTraits<Bytes> { static const FieldType type = FieldType::t_bytes; };

#define EASY_CODE(fieldType)\
	template<>struct FieldTypeTraits<fieldType> { static const FieldType type = FieldType::t_##fieldType; };

	EASY_CODE(int8_t)
		EASY_CODE(uint8_t)
		EASY_CODE(int16_t)
		EASY_CODE(uint16_t)
		EASY_CODE(int32_t)
		EASY_CODE(uint32_t)
		EASY_CODE(int64_t)
		EASY_CODE(uint64_t)
		EASY_CODE(double)
		EASY_CODE(float)

#undef EASY_CODE

	using TableFactorFun = std::function< std::unique_ptr<BaseTable> () >;
	struct Field
	{
		std::string name;
		FieldType type = FieldType::t_int8_t;
		KeyType keyType = KeyType::NONE;
		size_t pOffset = 0; //成员指针偏移
		size_t fieldSize = 0;//域的 sizeof
	};

	struct Table
	{
		std::string name;
		uint16_t tableId; //table类型唯一标识
		size_t pKeyOffset = 0; //主键指针偏移
		std::unordered_map<std::string,Field> m_allField;
		std::vector<Field> m_vecField; //根据定义先后顺序排放
		TableFactorFun factor = nullptr;
	};
	struct FieldInfo
	{
		void *fieldPoint=nullptr; //域变量的真正指针
		FieldType type= FieldType::t_int8_t;
	};
	class TableCfg
	{
	public:
		std::unordered_map<uint16_t, Table> m_allTable;

		static TableCfg &Ins()
		{
			static TableCfg d;
			return d;
		}
		TableCfg();

		bool GetFieldPoint(const BaseTable &obj, const std::string &fieldName, FieldInfo &fieldInfo);

		bool Pack(const BaseTable &obj, char *str, size_t &len);
		std::unique_ptr<BaseTable> Unpack(const char *str, size_t len);
	};
}