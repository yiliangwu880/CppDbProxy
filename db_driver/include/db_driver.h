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
#include "../src/log_def.h"

namespace db {

	class DbClientCon;

	using ConCb = std::function<void()>;

	//db driver 接口
	class BaseDbproxy: public Singleton<BaseDbproxy>
	{
		friend DbClientCon;

	private:
		ConCb m_conCb;
		std::unordered_map<uint16_t, void *> m_cmdId2Cb; //proto 消息ID 2 回调
		std::unordered_map<uint16_t, void *> m_id2QueryCb; //tableID 2 查询回调
		std::unordered_map<uint16_t, void *> m_id2InertCb;

	public:
		BaseDbproxy();
		void Init(const std::string &ip, uint16_t port, ConCb cb);
		bool Insert(const db::BaseTable &data);
		bool Update(const db::BaseTable &data);//更新数据，没填值的字段不会更新
		bool Query(const db::BaseTable &data, uint32 limit_num=1);
		bool Del(const db::BaseTable &data);
		//注册查询回调函数
		template<class DbTable>
		void RegQueryCb(void (*fun)(bool, const DbTable& ) )
		{
			DbTable t;
			m_id2QueryCb[t.TableId()] = (void *)fun;
		}
		template<class DbTable>
			void RegInsertCb(void(*fun)(bool, const DbTable&))
		{
			DbTable t;
			m_id2InertCb[t.TableId()] = (void *)fun;
		}

	private:
		//注册 proto消息回调
		template<class ProtoMsg, class Fun>
		void RegProtoParse(Fun fun)
		{
			ProtoMsg t;
			if (m_cmdId2Cb.find(t.id) != m_cmdId2Cb.end())
			{
				L_ERROR("repeated reg");
				return;
			}
			m_cmdId2Cb[t.id] = (void *)fun;
		}

		void OnRecv(const lc::MsgPack &msg);
		//@len 表示 msg 有效长度
		static void ParseInsert(const proto::insert_sc *msg, uint32_t len);
		static void ParseQuery(const proto::query_sc *msg, uint32_t len);
		static void ParseUpdate(const proto::update_sc *msg, uint32_t len);
		static void ParseDel(const proto::del_sc *msg, uint32_t len);
	};
}