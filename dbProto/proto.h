/*
"对象-关系映射"（Object/Relational Mapping）
*/
#pragma once
#include <string>
#include <vector>
#include <unordered_map>

#pragma pack(push)
#pragma pack(1)
namespace db {
	
	//约定协议前 uint16_t 为 cmdId.
	struct insert_cs
	{
		const uint16_t id = 1;
		uint16_t sid = 0;
		uint32_t dataLen; //data有效字节数
		char data[0]; //任意类型db 对象, 序列化的结果（会省略掉无内容的值，减少流量消耗）。
	};
	struct insert_sc
	{
		const uint16_t id = 2;
		uint16_t sid = 0;
		bool ret;
		uint32_t dataLen;
		char data[0]; //有值的内容作为条件
	};

	struct update_cs
	{
		const uint16_t id = 3;
		uint32_t dataLen;
		char data[0]; //没填值的字段不会更新
	};
	//struct update_sc
	//{
	//	const uint16_t id = 4;
	//	bool ret;
	//};

	
	struct query_cs
	{
		const uint16_t id = 5;
		uint16_t sid = 0;
		uint32_t limit_num = 1;
		bool isStr = false;		//true表示 data内容为 字符串条件，false表示 data内容为 任意类型db 对象, 序列化的结果
		uint16_t table_id = 0; //isStr==true的时候用保存 BaseTable::TableId()
		uint32_t dataLen = 0;
		char data[0]; //有值的内容作为条件
	};
	//多行结果，分多次 query_sc 协议发送
	struct query_sc
	{
		const uint16_t id = 6;
		uint16_t sid = 0;
		bool ret;
		uint32_t dataLen;
		//一个db 对象. 
		//失败,isStr==false情况，  返回请求的相同内容. 
		//失败,isStr==true的情况， 返回默认db obj,值都为0 or empty 
		char data[0]; 
	};

	struct del_cs
	{
		const uint16_t id = 7;
		uint16_t sid = 0;
		uint32_t dataLen;
		char data[0];//有值的内容作为条件
	};
	struct del_sc
	{
		const uint16_t id = 8;
		uint16_t sid = 0;
		bool ret;
		uint32_t dataLen;
		char data[0];//有值的内容作为条件
	};

	struct excute_sql_cs
	{
		const uint16_t id = 9;
		uint32_t sql_id;
		uint32_t dataLen;
		char data[0];//sql字符串
	};
	struct excute_sql_sc
	{
		const uint16_t id = 10;
		uint32_t sql_id;
	};
}
#pragma pack(pop)

/*

// reg id : fun, size
//parse
void parse(Cmd &cmd, size_t len)
{
	//cmd.id to fun
	//check len size
	info = map[cmd.id];
	info.fun(cmd);
}

template<class Insert>
struct FunTrait 
{
	using Fun = void InsertFun(Insert &msg);
};
template<class Insert, void *funAddr>
void fun<Insert>(Cmd &cmd)
{
	auto fun = (FunTrait<Insert>::Fun)funAddr;
	fun((Insert&)cmd);
}

void InsertFun(Insert &msg)
{
	//cmd.id to fun
}

map[3] = { fun<Insert>,InsertFun }  
//用模板获取函数类型，简化为
map[3] = InsertFun 


//////////////////////////send///
template<>
void send(Insert &cmd, DbTable db)
{
	::send(len)//len = cmd size + db size
	::send(cmd);
	::send(db);
}

void ParseQueryResultFun(QueryResult &msg)
{
	DbTable db = GetDb(msg);
}

*/