
#include "db_driver.h"
#include "log_def.h"
#include "connect.h"
#include "libevent_cpp/include/include_all.h"

using namespace std;
using namespace lc;
using namespace su;
using namespace db;
using namespace proto;

namespace
{
	//�� BaseTable���� �� Э��ṹProtoType  ������ MsgPack,������MsgPack::len��ProtoType �����ֶ�����δ��ֵ��
	//��������Ľṹ��dataLen data[0]��������壬���db ����
	//	struct query_sc
	//{
	//	const uint16_t id = 6;
	//	bool ret;  //��������
	//  uint32_t dataLen;
	//	char data[0]; //һ��db ����
	//};
	template<class ProtoType>
	ProtoType *BuildMsgPack(MsgPack &msg, const db::BaseTable &data)
	{
		//Ҫ��dataLen data[0]���������
		static_assert((size_t)&(((ProtoType *)(nullptr))->dataLen) == sizeof(ProtoType)-sizeof(uint32_t));
		static_assert((size_t)&(((ProtoType *)(nullptr))->data) == sizeof(ProtoType));
		static_assert((size_t)&(((ProtoType *)(nullptr))->id) == 0); //id�������ȶ���

		ProtoType *p = new (msg.data)ProtoType;
		//data���ó���
		size_t len = ArrayLen(msg.data) - sizeof(*p);
		if (!TableCfg::Ins().Pack(data, p->data, len))		{
			L_ERROR("pack fail");			return nullptr;		}		p->dataLen = len;		msg.len = sizeof(ProtoType) + len;		return p;	}
}

db::Dbproxy::Dbproxy()
{
	RegProtoParse(ParseInsert);
	RegProtoParse(ParseQuery);
	RegProtoParse(ParseDel);
}

void db::Dbproxy::Init(const std::string &ip, uint16_t port, ConCb cb)
{
	DbClientCon::Ins().ConnectInit(ip.c_str(), port);
	m_conCb = cb;
}

bool db::Dbproxy::Insert(const db::BaseTable &data)
{
	MsgPack msg;
	auto p = BuildMsgPack<insert_cs>(msg, data);
	L_COND(p, false);
	return DbClientCon::Ins().SendData(msg);
}

bool db::Dbproxy::Update(const db::BaseTable &data)
{
	MsgPack msg;
	auto p = BuildMsgPack<update_cs>(msg, data);
	L_COND(p, false);
	return DbClientCon::Ins().SendData(msg);
}

bool db::Dbproxy::Query(const db::BaseTable &data, uint32 limit_num/*=1*/)
{
	MsgPack msg;
	auto p = BuildMsgPack<query_cs>(msg, data);
	L_COND(p, false);
	p->limit_num = limit_num;
	return DbClientCon::Ins().SendData(msg);
}

bool db::Dbproxy::Del(const db::BaseTable &data)
{
	MsgPack msg;
	auto p = BuildMsgPack<del_cs>(msg, data);
	L_COND(p, false);
	return DbClientCon::Ins().SendData(msg);
}

void db::Dbproxy::OnRecv(const lc::MsgPack &msg)
{
	using ComFun = void(const char &); //��Ϣ�ص������� �������͡� 
	uint16_t cmdId = *(const uint16_t *)msg.data; //Լ��Э��ǰ uint16_t Ϊ cmdId. ���� 
	ComFun *fun = (ComFun *)(m_cmdId2Cb[cmdId]);
	const char *pMsg = &(msg.data[0]);
	(*fun)(*pMsg);
}

void db::Dbproxy::ParseInsert(const proto::insert_sc &msg)
{
	std::unique_ptr<BaseTable> pTable = TableCfg::Ins().Unpack(msg.data, msg.dataLen);
	L_COND_V(nullptr != pTable);

	using ComFun = void(bool, const BaseTable& ); //��ѯ�ص��� �������͡� 
	ComFun *fun = (ComFun *)(Dbproxy::Ins().m_id2InertCb[pTable->TableId()]);
	(*fun)(msg.ret, *(pTable.get()));
}

void db::Dbproxy::ParseQuery(const proto::query_sc &msg)
{
	std::unique_ptr<BaseTable> pTable = TableCfg::Ins().Unpack(msg.data, msg.dataLen);
	L_COND_V(nullptr != pTable);

	using ComFun = void(bool, const BaseTable&); //��ѯ�ص��� �������͡� 
	ComFun *fun = (ComFun *)(Dbproxy::Ins().m_id2QueryCb[pTable->TableId()]);
	(*fun)(msg.ret, *(pTable.get()));
}



void db::Dbproxy::ParseDel(const proto::del_sc &msg)
{
	std::unique_ptr<BaseTable> pTable = TableCfg::Ins().Unpack(msg.data, msg.dataLen);
	L_COND_V(nullptr != pTable);

	using ComFun = void(bool, const BaseTable&); //��ѯ�ص��� �������͡� 
	ComFun *fun = (ComFun *)(Dbproxy::Ins().m_id2DelCb[pTable->TableId()]);
	(*fun)(msg.ret, *(pTable.get()));
}
