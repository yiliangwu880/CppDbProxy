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
		UNIT_ASSERT(table.m_vecField.size() == 3);
		const Field f = table.m_vecField[0];
		UNIT_ASSERT(f.name == "id2");
		UNIT_ASSERT(f.type == FieldType::t_uint64_t);
		UNIT_ASSERT(f.keyType == KeyType::MAIN);
		UNIT_ASSERT(f.pOffset == sizeof(uint64_t) * 2);
		UNIT_ASSERT(f.fieldSize == sizeof(Player2::id2));
	}

	//check Player3
	{
		const Table &table = *(TableCfg::Ins().GetTable(3));
		UNIT_ASSERT(table.m_vecField.size() == 6);
		{
			const Field f = table.m_vecField[0];
			UNIT_ASSERT(f.name == "id");
			UNIT_ASSERT(f.type == FieldType::t_uint64_t);
			UNIT_ASSERT(f.keyType == KeyType::MAIN);
			UNIT_ASSERT(f.pOffset == sizeof(uint64_t) * 2);
			UNIT_ASSERT(f.fieldSize == sizeof(Player3::id));
		}
		{
			const Field f = table.m_vecField[1];
			UNIT_ASSERT(f.name == "id1");
			UNIT_ASSERT(f.type == FieldType::t_uint32_t);
			UNIT_ASSERT(f.keyType == KeyType::NONE);
			UNIT_ASSERT(f.pOffset == sizeof(uint64_t) * 2 + sizeof(uint64_t));
			UNIT_ASSERT(f.fieldSize == sizeof(Player3::id1));
		}
		{
			const Field f = table.m_vecField[2];
			UNIT_ASSERT(f.name == "id2");
			UNIT_ASSERT(f.type == FieldType::t_uint64_t);
			UNIT_ASSERT(f.keyType == KeyType::INDEX);
			UNIT_ASSERT(f.fieldSize == sizeof(Player3::id2));
		}
		{
			const Field f = table.m_vecField[3];
			UNIT_ASSERT(f.name == "myblob1");
			UNIT_ASSERT(f.type == FieldType::t_bytes);
			UNIT_ASSERT(f.keyType == KeyType::NONE);
			UNIT_ASSERT(f.fieldSize == 0);
		}
		{
			const Field f = table.m_vecField[4];
			UNIT_ASSERT(f.name == "id3");
			UNIT_ASSERT(f.type == FieldType::t_uint32_t);
			UNIT_ASSERT(f.keyType == KeyType::NONE);
			UNIT_ASSERT(f.fieldSize == sizeof(Player3::id3));
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

