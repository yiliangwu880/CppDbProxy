
#include "db_driver.h"
#include "log_def.h"
#include "connect.h"

using namespace std;
using namespace lc;
using namespace su;
using namespace db;
using namespace proto;

namespace
{
	//�� BaseTable���� �� Э��ṹ  ������ MsgPack��ProtoType �����ֶ�����δ��ֵ��
	//��������Ľṹ��data[0]����壬���db ����
	//	struct query_sc
	//{
	//	const uint16_t id = 6;
	//	bool ret;
	//	char data[0]; //һ��db ����
	//};
	template<class ProtoType>
	ProtoType *BuildMsgPack(MsgPack &msg, const db::BaseTable &data)
	{
		ProtoType *p = new (msg.data)ProtoType;
		//data���ó���
		size_t len = ArrayLen(msg.data) - sizeof(*p);
		if (!TableCfg::Ins().Pack(data, p->data, len))		{
			L_ERROR("pack fail");			return nullptr;		}		msg.len = sizeof(ProtoType) + len;		return p;	}
}


db::BaseDbproxy::BaseDbproxy()
{
	//RegProtoParse<insert_sc>(&BaseDbproxy::ParseInsert);
	//RegProtoParse<query_sc>(&BaseDbproxy::ParseQuery);

	RegProtoParse<decltype(BaseDbproxy::ParseInsert)>(BaseDbproxy::ParseInsert);
	//RegProtoParse<decltype(BaseDbproxy::ParseQuery)>(BaseDbproxy::ParseQuery);
}


void db::BaseDbproxy::Init(const std::string &ip, uint16_t port, ConCb cb)
{
	DbClientCon::Ins().ConnectInit(ip.c_str(), port);
	m_conCb = cb;
}

bool db::BaseDbproxy::Insert(const db::BaseTable &data)
{
	MsgPack msg;
	insert_cs *p = BuildMsgPack<insert_cs>(msg, data);
	L_COND(p, false);
	return DbClientCon::Ins().SendData(msg);
}

bool db::BaseDbproxy::Query(const db::BaseTable &data, uint32 limit_num/*=1*/)
{
	MsgPack msg;
	query_cs *p = BuildMsgPack<query_cs>(msg, data);
	L_COND(p, false);
	return DbClientCon::Ins().SendData(msg);
}

void db::BaseDbproxy::OnRecv(const lc::MsgPack &msg)
{
	using ComFun = void(const char &); //��Ϣ�ص������� �������͡� 
	uint16_t cmdId = *(const uint16_t *)msg.data; //Լ��Э��ǰ uint16_t Ϊ cmdId. ���� 
	ComFun *fun = (ComFun *)(m_cmdId2Cb[cmdId]);
	const char *pMsg = &(msg.data[0]);
	(*fun)(*pMsg);
}

void db::BaseDbproxy::ParseInsert(const proto::insert_sc &msg)
{
	std::unique_ptr<BaseTable> pTable = TableCfg::Ins().Unpack(msg.data, msg.dataLen);
	L_COND_V(nullptr != pTable);

	using ComFun = void(bool, const BaseTable& ); //��ѯ�ص��� �������͡� 
	ComFun *fun = (ComFun *)(BaseDbproxy::Ins().m_id2InertCb[pTable->TableId()]);
	BaseTable &table = *(pTable.get());
	(*fun)(msg.ret, table);
}

void db::BaseDbproxy::ParseQuery(const proto::query_sc &msg)
{

}
