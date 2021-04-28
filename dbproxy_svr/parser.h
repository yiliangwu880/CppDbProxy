/*
  解析并处理 svr ,db client的消息。
*/

#pragma once
#include "base_include.h"

class ProtoUtil
{
public:
	//测试例子
	static void Test();

	//获取字段选项
	static bool GetFieldOpt(const google::protobuf::Descriptor &des, const std::string &field_name, const std::string &opt_name, std::string &opt_value);
	static bool GetFieldOpt(const google::protobuf::Descriptor &des, const std::string &field_name, const std::string &opt_name, int32 &opt_value);
	static bool GetFieldOptEnum(const google::protobuf::Descriptor &des, const std::string &field_name, const std::string &opt_name, uint32 &opt_value);
	static bool GetFieldKeyOpt(const google::protobuf::Descriptor &des, const std::string &field_name, db::KeyType &key_type);

	//获取msg选项
	static bool GetMsgOpt(const google::protobuf::Descriptor &des, const std::string &opt_name, std::string &opt_value);
	//获取主键， 如果有，会从 num_key str_key其中之一返回.
	//@return false表示没有
	static bool GetMsgMainKeyVal(const google::protobuf::Message &msg, ::uint64 &num_key, std::string &str_key);
	static std::string GetMsgMainKeyName(const google::protobuf::Descriptor &des);

	//unique ptr返回，想泄漏都难。
	static std::unique_ptr<google::protobuf::Message> CreateMessage(const std::string &msg_name);
};