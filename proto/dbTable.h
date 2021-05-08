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
		t_struct, //任意struct 结构
	};
	//用户接口当string用，这里定义为了区分db 的 string 和 bytes
	struct Bytes : public  std::string
	{
	};
	
	//根据 真实类型 获取 FieldType::xx
	template<typename T>
	struct FieldTypeTraits
	{
		static const FieldType type = FieldType::t_struct;
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

	using PointChar = char*;
	using CPointChar = const char*;
	using PackFun = bool(*)(const char &t, PointChar &str, size_t &len); //t 是具体类型的抽象
	using UnpackFun = bool(*)(char &t, CPointChar &cur, size_t &len);
	struct Field
	{
		std::string name;
		FieldType type = FieldType::t_int32_t;
		KeyType keyType = KeyType::NONE;
		size_t pOffset = 0; //成员指针偏移
		size_t fieldSize = 0;//域的 sizeof
		int idx = 0;//定义先后索引，0开始
		PackFun packFun = nullptr;//非基础类型的域，需要独特的打包 解包 函数
		UnpackFun unpackFun = nullptr;
		//间接调用 packFun,提供std::string接口
		inline void Pack(const db::BaseTable &data, std::string &str) const
		{
			const char *pField = (char*)&data + pOffset;
			size_t len = 1024 * 2;
			str.resize(len);
			char *cur = (char *)str.c_str();
			packFun(*pField, cur, len);
			len = str.length() - len;
			str.resize(len);
		}
		inline void Unpack(db::BaseTable &data, const std::string &str) const
		{
			char *pField = (char*)&data + pOffset;
			const char *cur = str.c_str();
			size_t len = str.length();
			unpackFun(*pField, cur, len);
		}
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

		//@len [in] str 内存长度， [out] str 打包的有效长度
		bool Pack(const BaseTable &obj, char *str, size_t &len);
		std::unique_ptr<BaseTable> Unpack(const char *str, size_t len);
		const Table *GetTable(uint16_t tableId) const;
		const std::unordered_map<uint16_t, Table> &GetAllTable()const { return m_allTable; }
	private:
		void InitTableCfg();
		void CheckMissField();
		void CheckStructField();
	};
}