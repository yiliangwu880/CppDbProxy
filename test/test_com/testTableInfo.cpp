/*
brief: use example and test code
*/
#include "../com/unit_test.h"
#include <functional>
#include <limits>
#include <queue>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include "function_traits.h"
#include "db_driver/include/db_driver.h"
#include "external/svr_util/include/easy_code.h"
#include "external/svr_util/include/str_util.h"
#include "dbProto/dbStructPack.h"

using namespace std;
using namespace lc;
using namespace su;
using namespace db;
using namespace proto;


UNITTEST(testTableInfo)
{
	UNIT_ASSERT(TableCfg::Ins().GetTable(3) != nullptr);

	//check Player2
	{
		const Table &table = *(TableCfg::Ins().GetTable(2));
		UNIT_ASSERT(table.m_vecField.size() == 7);
		const Field f = table.m_vecField[0];
		UNIT_ASSERT(f.name == "id2");
		UNIT_ASSERT(f.type == FieldType::t_uint64_t);
		UNIT_ASSERT(f.keyType == KeyType::MAIN);
		UNIT_ASSERT(f.pOffset == sizeof(BaseTable));
		UNIT_ASSERT(f.fieldSize == sizeof(Player2::id2));
		UNIT_ASSERT(f.packFun == (db::PackFun)db::Pack<uint64_t>);
		UNIT_ASSERT(f.unpackFun == (db::UnpackFun)db::Unpack<uint64_t>);
	}

	//check Player3
	{
		const Table &table = *(TableCfg::Ins().GetTable(3));
		UNIT_ASSERT(table.m_vecField.size() == 8);
		{
			const Field f = table.m_vecField[0];
			UNIT_ASSERT(f.name == "id");
			UNIT_ASSERT(f.type == FieldType::t_uint64_t);
			UNIT_ASSERT(f.keyType == KeyType::MAIN);
			UNIT_ASSERT(f.pOffset == sizeof(BaseTable));
			UNIT_ASSERT(f.fieldSize == sizeof(Player3::id));
		}
		{
			const Field f = table.m_vecField[1];
			UNIT_ASSERT(f.name == "id1");
			UNIT_ASSERT(f.type == FieldType::t_uint32_t);
			UNIT_ASSERT(f.keyType == KeyType::NONE);
			UNIT_ASSERT(f.pOffset == sizeof(BaseTable) + sizeof(uint64_t));
			UNIT_ASSERT(f.fieldSize == sizeof(Player3::id1));
		}
		{
			struct A { uint64_t i;uint32_t i2; };
			const Field f = table.m_vecField[2];
			UNIT_ASSERT(f.name == "id2");
			UNIT_ASSERT(f.type == FieldType::t_uint64_t);
			UNIT_ASSERT(f.keyType == KeyType::INDEX);
			UNIT_ASSERT(f.pOffset == sizeof(BaseTable) + sizeof(A));
			UNIT_ASSERT(f.fieldSize == sizeof(Player3::id2));
		}
		{
			const Field f = table.m_vecField[3];
			UNIT_ASSERT(f.name == "myblob1");
			UNIT_ASSERT(f.type == FieldType::t_bytes);
			UNIT_ASSERT(f.keyType == KeyType::NONE);
			UNIT_ASSERT(f.fieldSize == 0);
			UNIT_ASSERT(f.packFun == (db::PackFun)db::Pack<Bytes>);
			UNIT_ASSERT(f.unpackFun == (db::UnpackFun)db::Unpack<Bytes>);
		}
		{
			const Field f = table.m_vecField[4];
			UNIT_ASSERT(f.name == "id3");
			UNIT_ASSERT(f.type == FieldType::t_uint32_t);
			UNIT_ASSERT(f.keyType == KeyType::NONE);
			UNIT_ASSERT(f.fieldSize == sizeof(Player3::id3));
			UNIT_ASSERT(f.packFun == (db::PackFun)db::Pack<uint32_t>);
			UNIT_ASSERT(f.unpackFun == (db::UnpackFun)db::Unpack<uint32_t>);
		} 
		{
			const Field f = table.m_vecField[5];
			UNIT_ASSERT(f.name == "myblob2");
			UNIT_ASSERT(f.type == FieldType::t_string);
			UNIT_ASSERT(f.keyType == KeyType::NONE);
			UNIT_ASSERT(f.fieldSize == 0);
		}

	}


	

}

