/*
//依赖libevent_cpp库
需要下面写才能工作：

main()
{

	调用本库的api


	EventMgr::Ins().Dispatch();
}

*/

#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <map>
#include "libevent_cpp/include/include_all.h"
#include "svr_util/include/singleton.h"
#include "svr_util/include/easy_code.h"
#include "svr_util/include/typedef.h"
#include "proto/dbTableDef.h"
#include "proto/proto.h"

namespace db {

	class DbClientCon;

	using ConCb = std::function<void()>;

	//db driver 接口
	class BaseDbproxy: public Singleton<BaseDbproxy>
	{
		friend DbClientCon;

	private:
		DbClientCon &m_con;
		ConCb m_conCb;
		std::unordered_map<uint16_t, void *> m_Id2QueryCb; //tableID 2 查询回调
		std::unordered_map<uint16_t, void *> m_Id2InertCb;

	public:
		BaseDbproxy();
		void Init(const std::string &ip, uint16_t port, ConCb cb);
		bool Insert(const db::BaseTable &dbObj);
		bool Update(const db::BaseTable &dbObj);//更新数据，没填值的字段不会更新
		bool Query(const db::BaseTable &dbObj, uint32 limit_num=1);
		bool Del(const db::BaseTable &dbObj);

		//注册查询回调函数
		template<class DbTable>
		void RegQueryCb(void (*fun)(bool, const DbTable& ) )
		{
			DbTable t;
			m_Id2QueryCb[t.TableId()] = (void *)fun;
		}
		template<class DbTable>
			void RegInsertCb(void(*fun)(bool, const DbTable&))
		{
			DbTable t;
			m_Id2InertCb[t.TableId()] = (void *)fun;
		}

	private:
		static void ParseInsert(const proto::insert_sc *msg);
		static void ParseQuery(const proto::query_sc *msg);
	};
}