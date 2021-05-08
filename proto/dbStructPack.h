/*
db table 定义，同时定义 c++对象类型
*/

#pragma once

#include <string>
#include <vector>
#include "../db_driver/src/log_def.h"
#include "dbTableDef.h"

namespace db
{
	//@cur [in]打包到内存地址，[out]第一个未使用的内存地址（下个变量打包的内存地址）。
	//@len [in]cur有效长度，[out]打包后，cur有效长度
	template<class T>
	inline bool Pack(const T &t, PointChar &cur, size_t &len)
	{
		L_ERROR("pack unknow. type name = %s", typeid(T).name());
		return false;
	}
	//@cur [in]解包来源内存地址，[out]第一个未解包的内存地址（下个变量解包的内存地址）。
	//@len [in]cur有效长度，[out]解包后，cur有效长度
	template<class T>
	inline bool Unpack(T &t, CPointChar &cur, size_t &len)
	{
		//static_assert(false);
		L_ERROR("unpack unknow. type name = %s", typeid(T).name());
		return false;
	}


#define EASY_CODE(T)\
	template<>\
	inline bool Pack<T>(const T &v, PointChar &cur, size_t &len)\
	{\
		L_COND(len >= sizeof(v), false);\
		*(T*)cur = v;\
		cur += sizeof(v);\
		len -= sizeof(v);\
		return true;\
	}\

	EASY_CODE(bool)
		EASY_CODE(int32_t)
		EASY_CODE(uint32_t)
		EASY_CODE(int64_t)
		EASY_CODE(uint64_t)
		EASY_CODE(double)
		EASY_CODE(int16_t)
		EASY_CODE(uint16_t)
		EASY_CODE(int8_t)
		EASY_CODE(uint8_t)

#undef  EASY_CODE

#define EASY_CODE(T)\
	template<>\
	inline bool Unpack<T>(T &v, CPointChar &cur, size_t &len)\
	{\
		L_COND(len >= sizeof(v), false);\
		v = *(T*)cur;\
		cur += sizeof(v);\
		len -= sizeof(v);\
		return true;\
	}\

		EASY_CODE(bool)
		EASY_CODE(int32_t)
		EASY_CODE(uint32_t)
		EASY_CODE(int64_t)
		EASY_CODE(uint64_t)
		EASY_CODE(double)

		EASY_CODE(int16_t)
		EASY_CODE(uint16_t)
		EASY_CODE(int8_t)
		EASY_CODE(uint8_t)
#undef  EASY_CODE
#if 0 //test code
		using TEST_TYPE =uint64_t;
		template<>
		inline bool Pack<TEST_TYPE>(const TEST_TYPE &v, PointChar &cur, size_t &len)
		{
			L_DEBUG("Pack<TEST_TYPE>");
			L_COND(len >= sizeof(v), false);

			L_DEBUG("v = %ld", v);
			*(TEST_TYPE*)cur = v;
			L_DEBUG("size = %d", sizeof(v));
			L_DEBUG("%d %d %d %d %d %d %d %d", *cur, *(cur + 1), *(cur + 2), *(cur + 3), *(cur + 4), *(cur + 5), *(cur + 6), *(cur + 7));

			cur += sizeof(v);
			len -= sizeof(v);

			return true;
		}

		template<>
		inline bool Unpack<TEST_TYPE>(TEST_TYPE &v, CPointChar &cur, size_t &len)
		{
			L_DEBUG("Unpack<TEST_TYPE>");
			{
				L_DEBUG("cur = %p ", cur);
			}
			L_COND(len >= sizeof(v), false);
			v = *(TEST_TYPE*)cur;
			cur += sizeof(v);
			len -= sizeof(v);
			L_DEBUG("%d", v);
			return true;
		}
#endif


	template<>
	inline bool Pack<std::string>(const std::string &v, PointChar &cur, size_t &len)
	{
		uint32_t strLen = v.length();
		L_COND(Pack(strLen, cur, len),false);
		L_COND(len >= v.length(), false);
		memcpy(cur, v.c_str(), v.length());
		cur += v.length();
		len -= v.length();
		return true;
	}
	template<>
	inline bool Unpack<std::string>(std::string &v, CPointChar &cur, size_t &len)
	{
		uint32_t strLen;
		L_COND(Unpack(strLen, cur, len), false);
		L_COND(len >= strLen, false);
		v.assign(cur, strLen);
		cur += strLen;
		len -= strLen;
		return true;
	}

	template<>
	inline bool Pack<Bytes>(const Bytes &v, PointChar &cur, size_t &len)
	{
		return Pack<std::string>(v, cur, len);
	}
	template<>
	inline bool Unpack<Bytes>(Bytes &v, CPointChar &cur, size_t &len)
	{
		return Unpack<std::string>(v, cur, len);
	}

	template<class Vec>
	inline bool PackVector(const Vec &v, PointChar &cur, size_t &len)
	{
		uint32_t vecLen = v.size();
		L_COND(Pack(vecLen, cur, len), false);
		for (const auto &vt : v)
		{
			L_COND(Pack(vt, cur, len), false);
		}
		return true;
	}

	template<class Vec>
	inline bool UnpackVector(Vec &v, CPointChar &cur, size_t &len)
	{
		uint32_t vecLen;
		L_COND(Unpack(vecLen, cur, len), false);
		v.resize(vecLen);
		for (uint32_t i = 0; i < vecLen; i++)
		{
			L_COND(Unpack(v[i], cur, len), false);
		}
		return true;
	}

	//定义所有vector<xx>
#define EASY_CODE(T)\
	template<>\
	inline bool Pack<std::vector<T>>(const std::vector<T> &v, PointChar &cur, size_t &len)\
	{\
		return PackVector(v, cur, len);\
	}\
	template<>\
	inline bool Unpack<std::vector<T>>(std::vector<T> &v, CPointChar &cur, size_t &len)\
	{\
		return UnpackVector(v, cur, len);\
	}\

		EASY_CODE(int32_t)
		EASY_CODE(uint32_t)
		EASY_CODE(int64_t)
		EASY_CODE(uint64_t)
		EASY_CODE(double)
		EASY_CODE(int16_t)
		EASY_CODE(uint16_t)
		EASY_CODE(int8_t)
		EASY_CODE(uint8_t)

#undef  EASY_CODE


///////////////////////自定义结构体////////////////////////////
#if 0
	//宏代码模板
	template<>
	inline bool Pack<Ride>(const Ride &v, PointChar &cur, size_t &len)
	{
		L_DEBUG("Pack<Ride>");
		L_COND(Pack(v.id, cur, len), false);
		L_COND(Pack(v.ids, cur, len), false);
		return true;
	}

	template<>
	inline bool Unpack<Ride>(Ride &v, CPointChar &cur, size_t &len)
	{
		L_DEBUG("Unpack<Ride>");
		L_COND(Unpack(v.id, cur, len), false);
		L_COND(Unpack(v.ids, cur, len), false);
		return true;
	}
#endif

#if 1
#define DB_CLASS_NAME(className)\
	template<>\
	inline bool Pack<className>(const className &v, PointChar &cur, size_t &len)\
	{\

#define DB_FIELD(fieldName)\
	L_COND(Pack(v.fieldName, cur, len), false);\

#define DB_CLASS_END\
		return true;\
	}\

	DB_ALL_STRUCT_INFO

#undef  DB_CLASS_NAME
#undef  DB_FIELD
#undef  DB_CLASS_END

#define DB_CLASS_NAME(className)\
	template<>\
	inline bool Unpack<className>(className &v, CPointChar &cur, size_t &len)\
	{\

#define DB_FIELD(fieldName)\
	L_COND(Unpack(v.fieldName, cur, len), false);\

#define DB_CLASS_END\
		return true;\
	}\

		DB_ALL_STRUCT_INFO

#undef  DB_CLASS_NAME
#undef  DB_FIELD
#undef  DB_CLASS_END
#endif
}