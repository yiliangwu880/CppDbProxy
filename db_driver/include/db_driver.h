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
#include "svr_util/include/singleton.h"
#include "svr_util/include/easy_code.h"
#include "svr_util/include/typedef.h"
#include "dbProto/dbTableDef.h"
#include "dbProto/proto.h"
#include "../src/log_def.h"

namespace lc
{
	class MsgPack;
}
namespace db {

	class DbClientCon;

	using ConCb = std::function<void()>;
	using ExcuteSqlCb = std::function<void(uint32_t)>;

	//db driver 接口
	class Dbproxy: public Singleton<Dbproxy>
	{
		friend DbClientCon;

	private:
		ConCb m_conCb;
		ExcuteSqlCb m_sqlCb;
		std::unordered_map<uint16_t, void *> m_cmdId2Cb; //proto 消息ID 2 回调
		std::unordered_map<uint16_t, void *> m_id2QueryCb; //tableID 2 查询回调
		std::unordered_map<uint16_t, void *> m_id2InertCb;
		std::unordered_map<uint16_t, void *> m_id2DelCb;

	public:
		Dbproxy();
		void Init(const std::string &ip, uint16_t port, ConCb cb=nullptr, ExcuteSqlCb SqlCb = nullptr);
		bool Insert(const db::BaseTable &data);
		bool Update(const db::BaseTable &data);//更新数据，没填值的字段不会更新
		bool Query(const db::BaseTable &data, uint32 limit_num = 1);
		//@data 用来识别类型用，值不读取
		bool Query(const db::BaseTable &data, const std::string &cond, uint32 limit_num = 1);
		bool Del(const db::BaseTable &data);
		void ExecuteSql(const std::string &sql, uint32_t sql_id=0);
		//注册查询回调函数
		//DbTable 为 db::BaseTable的派生类
		template<class DbTable>
		void RegQueryCb(void (*fun)(bool, const DbTable& ) )
		{
			DbTable t;
			if (m_id2QueryCb.find(t.TableId()) != m_id2QueryCb.end())
			{
				L_ERROR("repeated reg");
				return;
			}
			m_id2QueryCb[t.TableId()] = (void *)fun;
		}
		template<class DbTable>
			void RegInsertCb(void(*fun)(bool, const DbTable&))
		{
			DbTable t;
			if (m_id2InertCb.find(t.TableId()) != m_id2InertCb.end())
			{
				L_ERROR("repeated reg");
				return;
			}
			m_id2InertCb[t.TableId()] = (void *)fun;
		}
		template<class DbTable>
		void RegDelCb(void(*fun)(bool, const DbTable&))
		{
			DbTable t;
			if (m_id2DelCb.find(t.TableId()) != m_id2InertCb.end())
			{
				L_ERROR("repeated reg");
				return;
			}
			m_id2DelCb[t.TableId()] = (void *)fun;
		}

	private:
		//注册 proto消息回调
		template<class MsgType> 
		void RegProtoParse(void(*fun)(const MsgType &msg))
		{
			//typedef typename std::function<Fun>::argument_type MsgRefType;
			//typedef typename std::remove_reference<MsgRefType>::type MsgConstType;
			//typedef typename std::remove_const<MsgConstType>::type Type;
			MsgType msg;
			if (m_cmdId2Cb.find(msg.id) != m_cmdId2Cb.end())
			{
				L_ERROR("repeated reg");
				return;
			}
			m_cmdId2Cb[msg.id] = (void *)fun;
		}

		void OnRecv(const lc::MsgPack &msg);
		//@len 表示 msg 有效长度
		static void ParseInsert(const insert_sc &msg);
		static void ParseQuery(const query_sc &msg);
		static void ParseDel(const del_sc &msg);
		static void ParseExcuteSql(const excute_sql_sc &msg);
	};
}