/*
"对象-关系映射"（Object/Relational Mapping）
*/
#pragma once
#include <string>
#include <vector>
#include <unordered_map>

#pragma pack(push)
#pragma pack(1)
namespace proto {
	
	static uint16_t Id()
	{
		static uint16_t id = 1;
		return id++;
	}
	struct insert_cs
	{
		const uint16_t id = Id();
		char dbObj[0]; //任意类型db 对象, 序列化的结果（会省略掉无内容的值，减少流量消耗）。
	};
	struct insert_sc
	{
		const uint16_t id = Id();
		bool ret;
		char dbObj[0]; //有值的内容作为条件
	};

	struct update_cs
	{
		const uint16_t id = Id();
		char dbObj[0]; //没填值的字段不会更新
	};
	struct update_sc
	{
		const uint16_t id = Id();
		bool ret;
	};

	struct query_cs
	{
		const uint16_t id = Id();
		uint16_t limit_num;
		char dbObj[0]; //有值的内容作为条件
	};
	struct query_sc
	{
		const uint16_t id = Id();
		bool ret;
		char dbObj[0]; 
	};

	struct del_cs
	{
		const uint16_t id = Id();
		char dbObj[0];//有值的内容作为条件
	};
	struct del_sc
	{
		const uint16_t id = Id();
		bool ret;
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