#include "parser.h"

using namespace su;
using namespace lc;
using namespace std;
using namespace google::protobuf;
using namespace db;

void ProtoUtil::Test()
{
	TestMsg msg;
	const Descriptor *dec = msg.GetDescriptor();
	printf("------------------s11-----------------\n");
	{
		string v;
		ProtoUtil::GetMsgOpt(*dec, "db.OptMsg", v);
		printf("OptMsg:%s\n", v.c_str());
	}
	{
		string v;
		ProtoUtil::GetFieldOpt(*dec, "s2", "db.OptStr", v);
		printf("OptStr:%s\n", v.c_str());
	}
	{
		int32 v;
		ProtoUtil::GetFieldOpt(*dec, "s3", "db.OptInt", v);
		printf("OptInt:%d\n", v);
	}
	{
		uint32 v;
		ProtoUtil::GetFieldOptEnum(*dec, "id", "db.OptKey", v);
		printf("OptKey:%d\n", v);
	}
}


bool ProtoUtil::GetFieldOpt(const google::protobuf::Descriptor &des, const std::string &field_name, const std::string &opt_name, string &v)
{
	const FieldDescriptor* fd = des.FindFieldByName(field_name);
	COND_F(fd);
	const FieldOptions& fo = fd->options();
	const Reflection* ref = fo.GetReflection();
	COND_F(ref);
	const FieldDescriptor* opt_fd = ref->FindKnownExtensionByName(opt_name);
	COND_F(opt_fd);

	v = ref->GetString(fo, opt_fd);
	return true;
}

bool ProtoUtil::GetFieldOpt(const google::protobuf::Descriptor &des, const std::string &field_name, const std::string &opt_name, int32 &v)
{
	const FieldDescriptor* fd = des.FindFieldByName(field_name);
	COND_F(fd);
	const FieldOptions& fo = fd->options();
	const Reflection* ref = fo.GetReflection();
	COND_F(ref);
	const FieldDescriptor* opt_fd = ref->FindKnownExtensionByName(opt_name);
	COND_F(opt_fd);

	v = ref->GetInt32(fo, opt_fd);
	return true;
}

bool ProtoUtil::GetFieldOptEnum(const Descriptor &des, const std::string &field_name, const std::string &opt_name, uint32 &v)
{
	const FieldDescriptor* fd = des.FindFieldByName(field_name);
	COND_F(fd);
	const FieldOptions& fo = fd->options();
	const Reflection* ref = fo.GetReflection();
	COND_F(ref);
	const FieldDescriptor* opt_fd = ref->FindKnownExtensionByName(opt_name);
	COND_F(opt_fd);

	v = ref->GetEnumValue(fo, opt_fd);
	return true;
}

bool ProtoUtil::GetFieldKeyOpt(const Descriptor &des, const std::string &field_name, KeyType &key_type)
{
	uint32 v;
	bool r = GetFieldOptEnum(des, field_name, "db.OptKey", v);
	key_type = (KeyType)v;
	return r;
}

bool ProtoUtil::GetMsgOpt(const google::protobuf::Descriptor &des, const std::string &opt_name, std::string &v)
{
	const MessageOptions& opt = des.options();
	const Reflection* ref = opt.GetReflection();
	COND_F(ref);
	const FieldDescriptor* opt_fd = ref->FindKnownExtensionByName(opt_name);
	COND_F(opt_fd);

	v = ref->GetString(opt, opt_fd);
	return true;
}

bool ProtoUtil::GetMsgMainKeyVal(const google::protobuf::Message &msg, ::uint64 &num_key, std::string &str_key)
{
	str_key.clear();
	num_key = 0;

	const Descriptor *des = msg.GetDescriptor();
	int cnt = des->field_count();
	L_COND_F(cnt > 0, "no field");
	for (int i = 0; i < cnt; i++)
	{
		const FieldDescriptor* field = des->field(i);
		L_COND_F(field);
		KeyType key_type;
		GetFieldKeyOpt(*des, field->name(), key_type);
		if (db::K_MAIN_KEY != key_type)
		{
			continue;
		}

		//is main key
		const Reflection* ref = msg.GetReflection();
		L_COND_F(ref);
		if (FieldDescriptor::TYPE_STRING == field->type())
		{
			str_key = ref->GetString(msg, field);
			return true;
		}
		else//其他一致认为是uint64处理
		{
			num_key = ref->GetUInt64(msg, field);
			return true;
		}
	}
	return false;
}

std::string ProtoUtil::GetMsgMainKeyName(const google::protobuf::Descriptor &des)
{
	int cnt = des.field_count();
	L_COND_F(cnt > 0, "no field");
	for (int i = 0; i < cnt; i++)
	{
		const FieldDescriptor* field = des.field(i);
		L_COND_F(field);
		KeyType key_type;
		GetFieldKeyOpt(des, field->name(), key_type);
		if (db::K_MAIN_KEY != key_type)
		{
			continue;
		}
		//is main key
		return field->name();
	}
	return "";
}

std::unique_ptr<Message> ProtoUtil::CreateMessage(const std::string &msg_name)
{
	unique_ptr<Message> p;
	const Descriptor* des = DescriptorPool::generated_pool()->FindMessageTypeByName(msg_name);
	COND_R(des, nullptr);
	const Message* prototype = MessageFactory::generated_factory()->GetPrototype(des);
	COND_R(prototype, nullptr);
	p.reset(prototype->New());
	return p;
}


