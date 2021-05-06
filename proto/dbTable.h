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
		const char *className = nullptr;
		uint16_t tableId; //table类型唯一标识
	public:
		friend class TableCfg;
		BaseTable(const char *name, uint16_t id) : className(name), tableId(id){}
		virtual ~BaseTable() {}; //必须要虚析构函数，才能让 unique_ptr<BaseTable> 正确释放。
		uint16_t TableId() const { return tableId; }
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
		t_int32_t,
		t_uint32_t,
		t_int64_t,
		t_uint64_t,
		t_double,
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
		static const FieldType type = FieldType::t_uint32_t;
	};

	template<>struct FieldTypeTraits<std::string> { static const FieldType type = FieldType::t_string; };
	template<>struct FieldTypeTraits<Bytes> { static const FieldType type = FieldType::t_bytes; };

#define EASY_CODE(fieldType)\
	template<>struct FieldTypeTraits<fieldType> { static const FieldType type = FieldType::t_##fieldType; };

	EASY_CODE(bool)
		EASY_CODE(int32_t)
		EASY_CODE(uint32_t)
		EASY_CODE(int64_t)
		EASY_CODE(uint64_t)
		EASY_CODE(double)

#undef EASY_CODE

	//根据 FieldType::xx 获取 真实类型
	template<FieldType T>
	struct RealTypeTraits
	{
	};

	template<>struct RealTypeTraits<FieldType::t_string> { using Type = std::string; };

#define EASY_CODE(fieldType)\
	template<>struct RealTypeTraits<FieldType::t_##fieldType> { using Type = fieldType; };

	EASY_CODE(bool)
		EASY_CODE(int32_t)
		EASY_CODE(uint32_t)
		EASY_CODE(int64_t)
		EASY_CODE(uint64_t)
		EASY_CODE(double)

#undef EASY_CODE

	using TableFactorFun = std::function< std::unique_ptr<BaseTable> () >;
	struct Field
	{
		std::string name;
		FieldType type = FieldType::t_int32_t;
		KeyType keyType = KeyType::NONE;
		size_t pOffset = 0; //成员指针偏移
		size_t fieldSize = 0;//域的 sizeof
		int idx = 0;//定义先后索引，0开始
	};

	struct Table
	{
		std::string name;
		uint16_t tableId; //table类型唯一标识
		//std::unordered_map<std::string,Field> m_allField;
		std::vector<Field> m_vecField; //根据定义先后顺序排放
		TableFactorFun factor = nullptr;

		const Field *GetMainKey() const;
	};
	struct FieldInfo
	{
		void *fieldPoint=nullptr; //域变量的真正指针
		FieldType type= FieldType::t_uint32_t;
	};
	class TableCfg
	{
		std::unordered_map<uint16_t, Table> m_allTable;
	public:
		static TableCfg &Ins()
		{
			static TableCfg d;
			return d;
		}
		TableCfg();

		//bool GetFieldPoint(const BaseTable &obj, const std::string &fieldName, FieldInfo &fieldInfo);

		bool Pack(const BaseTable &obj, char *str, size_t &len);
		std::unique_ptr<BaseTable> Unpack(const char *str, size_t len);
		const Table *GetTable(uint16_t tableId) const;
		const std::unordered_map<uint16_t, Table> &GetAllTable()const { return m_allTable; }
	private:
		void InitTableCfg();
		void CheckMissField();

	};
}