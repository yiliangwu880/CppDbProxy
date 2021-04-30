
//this file is build by CppCfg Tool
//don't modify by manual
#pragma once
#include <string>
#include <array>
#include "SimpleCfg.h"

struct Cfg
{
	struct S_mongodb_db
	{
		std::string db_ip;
		std::string db_name;
		uint16 db_port;
		std::string db_psw;
		std::string db_user;
	};
	struct S_mysql_db
	{
		std::string db_ip;
		std::string db_name;
		uint16 db_port;
		std::string db_psw;
		std::string db_user;
	};


	////////////////////////define member list////////////////////////
	std::string ip;
	uint16 is_daemon;
	S_mongodb_db mongodb_db;
	S_mysql_db mysql_db;
	uint16 port;
	std::string select_db;

	////////////////////////method list////////////////////////
	//load or reload cfg file .
	bool LoadFile(const char *file_name=nullptr)
	{
		//default load original cfg file name
		if (nullptr == file_name)
		{
			file_name = "../../bin/dbproxy_svr/Cfg.txt";
		}
		SimpleCfg js;
		if (!js.ParseFile(file_name))
		{
			return false;
		}
		return Assign(js);
	}

private:
	template<typename Array>
	inline size_t ArrayLen(const Array &array)
	{
		return sizeof(array) / sizeof(array[0]);
	}
	//initialize or reload cfg content.
	bool Assign(const nlohmann::json &js)
	{
		try
		{

			ip = js["ip"];
			is_daemon = js["is_daemon"];
			mongodb_db.db_ip = js["mongodb_db"]["db_ip"];
			mongodb_db.db_name = js["mongodb_db"]["db_name"];
			mongodb_db.db_port = js["mongodb_db"]["db_port"];
			mongodb_db.db_psw = js["mongodb_db"]["db_psw"];
			mongodb_db.db_user = js["mongodb_db"]["db_user"];
			mysql_db.db_ip = js["mysql_db"]["db_ip"];
			mysql_db.db_name = js["mysql_db"]["db_name"];
			mysql_db.db_port = js["mysql_db"]["db_port"];
			mysql_db.db_psw = js["mysql_db"]["db_psw"];
			mysql_db.db_user = js["mysql_db"]["db_user"];
			port = js["port"];
			select_db = js["select_db"];


			return true;
		}
		catch (...)
		{
			//if fail, pls check if your cfg fomart is legal.
			return false;
		}
	}
};
