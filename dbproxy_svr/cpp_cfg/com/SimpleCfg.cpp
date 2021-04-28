#include "SimpleCfg.h"
#include <sstream>
#include <limits>
#include <fstream>
#include <stdarg.h>

using uint16 = unsigned short;
using uint32 = unsigned int;
using uint64 = unsigned long long;
using int64 = long long;
using uint8 = unsigned char;
using int32 = int;

using json = nlohmann::json;
using namespace std;
using namespace SimpleCfgLog;

namespace SimpleCfgLog
{
	static bool g_enable_log = false;
	bool IsEnableLog()
	{
		return g_enable_log;
	}

	void EnableLog(bool enable_log)
	{
		g_enable_log = enable_log;
	}

	void Log(const char * pattern, ...)
	{
		if (!g_enable_log)
		{
			return;
		}
		va_list vp;
		va_start(vp, pattern);
		{
			std::string s;
			s.append(pattern);
			s.append("\n");

			vprintf(s.c_str(), vp);
		}
		va_end(vp);
	}

}
namespace
{
	string Js2String(json &js)
	{
		std::stringstream ss;
		ss << js;

		return ss.str();
	}
	//return -1表示失败，成功返回索引
	template<class T>
	int FindValueInArray(const json &js, const T &value)
	{
		if (!js.is_array())
		{
			return -1;
		}
		for (int i=0; i<js.size(); ++i)
		{
			if (js[i] == value)
			{
				return i;
			}
		}
		return -1;
	}

	const char * NumToStr(int num)
	{
		static char buf[100];
		snprintf(buf, sizeof(buf), "%d", num);
		return buf;
	}
}

bool SimpleCfg::ParseFile(const std::string &file_name)
{
	std::ifstream file(file_name);
	std::string str((std::istreambuf_iterator<char>(file)),
		std::istreambuf_iterator<char>());
	file.close();                    // close file handle

	if (!ParseStr(str))
	{
		Log("parse file fail.");
		return false;
	}
	return true;
}

bool SimpleCfg::ParseStr(const string &cfg)
{
	try
	{
		if (!ParseObj(cfg.cbegin(), cfg.cend(), *this))
		{
			Log("parse fail");
			return false;
		}
		if (!IsEnableObj(*this))
		{
			Log("error, json format can't build cpp");
			return false;
		}
		Log("parse ok");
		return true;
	}
	catch (exception& e)
	{
		Log("parse json fail. exception %s", e.what());
	}
	catch (...)
	{
		Log("parse json fail");
	}
	return false;
}

const char *SimpleCfg::c_str()
{
	std::stringstream ss;
	ss << *this;
	static string str;
	str.assign(ss.str());
	return str.c_str();
}



const char * SimpleCfg::DynamicStr = "dynamic";

bool SimpleCfg::IsEnableObj(const nlohmann::json &js)

{
	if (!js.is_object())
	{
		Log("error, para is not object");
		return false;
	}

	const json *dynArray = nullptr;
	{
		auto it = js.find(SimpleCfg::DynamicStr);
		if (it != js.end())
		{
			dynArray = &js[SimpleCfg::DynamicStr];
		}
	}

	for (auto &el : js.items())
	{
		const json &member = el.value();
		const string &key = el.key();
		if (nullptr != dynArray)
		{
			//for (auto& v : *dynArray)
			auto it = dynArray->find(key);
			if (it != dynArray->end())
			{
				continue; //忽略检查动态对象
			}
		}

		if (member.is_object())
		{
			if (!IsEnableObj(member))
			{
				return false;
			}
		}
		else if (member.is_array())
		{
			if (!IsEnableArray(member))
			{
				return false;
			}
		}
		//base member is always enable
	}
	return true;
}

bool SimpleCfg::IsEnableArray(const nlohmann::json &js)
{
	if (!js.is_array())
	{
		Log("error para, js is not array");
		return false;
	}
	json js_1st;
	for (const json &member : js)
	{
		if (js_1st.is_null())
		{
			js_1st = member;
		}
		else
		{
			if (js_1st.type() != member.type() )
			{
				if (!(js_1st.is_number() && member.is_number()))
				{
					Log("error , SimpleCfg array element must be same type");
					return false;
				}
			}
		}
	}
	return true;
}
//字符串解析成json对象
//@start, end 字符串迭代器，错误用户自己负责
bool SimpleCfg::ParseObj(CharIter start, CharIter end, json &js)
{
	if (start == end)
	{
		Log("error, empty cfg string");
		return false;
	}

	//迭代每个成员变量
	CharIter cur = start; //指向当前待分析的字符串开始位置
	for (uint32 n = 0; n < 50000; ++n)
	{
		//处理完成
		if (cur == end)
		{
			return true;
		}

		string name, value;
		bool isDynamic=false;
		if (!Find1stMemStr(cur, end, name, value, isDynamic))
		{
			return true;//没有成员了
		}
		if (isDynamic)
		{
			js[DynamicStr].push_back(name);
		}

		if (!js[name].is_null())
		{
			Log("error , repeated menber name");
			return false;
		}
		L_S_DEBUG("find member and value: [%s] = [%s]", name.c_str(), value.c_str());
		//到这里， cur 已经指向下一个成员字符串开头
		if (value[0] == '{')
		{
			json v;
			if (!ParseObj(value.cbegin()+1, value.cend(), v))
			{
				return false;
			}
			js[name] = v;
		}
		else if(value[0] == '[')
		{
			json v;
			if (!ParseArray(value.cbegin(), value.cend(), v))
			{
				return false;
			}
			js[name] = v;
		}
		else 
		{
			json v;
			if (!ParseBaseValue(value, v))
			{
				return false;
			}
			L_S_DEBUG("ParseBaseValue result js=%s", Js2String(v).c_str());
			js[name] = v;
			L_S_DEBUG("parent result js=%s", Js2String(js).c_str());
		}
	}
	Log("error, ParseObj endless loop");
	return false;
}

bool SimpleCfg::ParseArray(CharIter start, CharIter end, json &js)
{
	if (start == end)
	{
		Log("error, empty cfg string");
		return false;
	}
	js.clear();

	//迭代每个元素变量
	CharIter cur = start+1; //指向当前待分析的字符串开始位置,跳过[
	for (uint32 n = 0; n < 50000; ++n)
	{
		//处理完成
		if (cur == end)
		{
			return true;
		}

		string value;
		if (!Find1stElment(cur, end, value))
		{
			return true;//没有元素了
		}
		L_S_DEBUG("find elment = %s", value.c_str());
		//到这里， cur 已经指向下一个成员字符串开头

		if (value[0] == '{')
		{
			json v;
			if (!ParseObj(value.cbegin()+1, value.cend(), v))
			{
				return false;
			}
			js.push_back(v);
		}
		else if (value[0] == '[')
		{
			json v;
			if (!ParseArray(value.cbegin(), value.cend(), v))
			{
				return false;
			}
			js.push_back(v);
		}
		else
		{
			json v;
			if (!ParseBaseValue(value, v))
			{
				return false;
			}
			js.push_back(v);
		}
	}
	Log("error, ParseObj endless loop");
	return false;
}

bool SimpleCfg::CheckAndJumpSpace(CharIter &cur, CharIter end)
{
	if (cur == end)
	{
		return false;
	}
	//跳过分隔符
	bool isFind = false;
	for (uint32 n = 0; n < 50000; ++n)
	{
		if (*cur==' ')
		{
			isFind = true;
			cur++;
			if (cur == end)
			{
				break;
			}
		}
		else
		{
			break;
		}
		if (n >= 50000 - 1)
		{
			Log("error, CheckAndJumpSplit endless loop ");
			return false;
		}
	}
	return isFind;
}
bool SimpleCfg::CheckAndJumpSplit(CharIter &cur, CharIter end)
{
	if (cur == end)
	{
		return false;
	}
	//跳过分隔符
	bool isFind = false;
	for (uint32 n = 0; n < 50000; ++n)
	{
		if (IsSplitChar(*cur))
		{
			isFind = true;
			cur++; 	
			if (cur == end)
			{
				break;
			}
		}
		else
		{
			break;
		}
		if (n >= 50000 - 1)
		{
			Log("error, CheckAndJumpSplit endless loop ");
			return false;
		}
	}
	return isFind;
}

void SimpleCfg::CheckAndJumpSCS(CharIter &cur, CharIter end)
{
	if (cur == end)
	{
		return;
	}
	auto start = cur;
	for (uint32 n = 0; n < 50000; ++n)
	{
		if (CheckAndJumpSplit(cur, end))
		{
			continue;
		}
		if (CheckAndJumpComment(cur, end))
		{
			continue;
		}
		if (CheckAndJumpSpace(cur, end))
		{
			continue;
		}
		break;
		if (n >= 50000 - 1)
		{
			Log("error, CheckAndJumpSplitComment endless loop  %d [%s] [%s]", n, &*start, &*cur);
			return;
		}
	}
}

bool SimpleCfg::CheckAndJumpComment(CharIter &cur, CharIter end)
{
	if (cur == end)
	{
		return false;
	}
	//cur 跳过注释 "//"
	if ('/' == *cur && cur != end)
	{
		if ('/' != *(cur))
		{
			return false;
		}

		//有注释 "//"
		L_S_DEBUG("find comment //");
		CharIter it = std::find(cur, end, '\n');
		if (it == end)
		{
			cur = end;
			L_S_DEBUG("jump to =%s", &*cur);
			return true;
		}
		cur = it + 1;
		L_S_DEBUG("jump to =%s", &*cur);
		return true;
	}
	return false;
}

//解析基础值，
//@js [out] 值赋值给js=xx
bool SimpleCfg::ParseBaseValue(const string &value_in, json &js)
{
	if (value_in.empty())
	{
		Log("error para in ParseBaseValue");
		return false;
	}
	try
	{
		CharIter cur = value_in.begin();
		CheckAndJumpSCS(cur, value_in.end());
		string value(cur,value_in.end());
		L_S_DEBUG("ParseBaseValue %s", value.c_str());

		{//运算符的情况
			CharIter it = std::find_if(value.cbegin(), value.cend(), IsOperatorChar);
			if (it != value.cend() && it != value.cbegin())//有操作符而且操作符不是字符前
			{
				Log("handler operator");
				if ((it+1)== value.cend())
				{
					Log("error, value end can't be operator. value=[%s]", value.c_str());
					return false;
				}
				string left_num(value.cbegin(), it);
				json left_js;
				//L_S_DEBUG("left_num=%s", left_num.c_str());
				ParseBaseValue(left_num, left_js);

				string right_num(it + 1, value.cend());
				json right_js;
				//L_S_DEBUG("right_num=%s", right_num.c_str());
				ParseBaseValue(right_num, right_js);
				js=OperatorBaseValue(left_js, *it, right_js);
				return true;
			}
		}
		//大写库不支持，特殊处理
		{
			if (value == "TRUE"
				)
			{
				js = true;
				return true;
			}
			if (value == "FALSE")
			{
				js = false;
				return true;
			}
		}
		//16进制
		if (
			value.size() > 2
			&& value[0] == '0'
			&& (value[1] == 'x' || value[1] == 'X')
			)
		{
			char *endptr = nullptr;
			int64 v = strtoll(&value[2], &endptr, 16);
			js = v;
			return true;
		}
		//16进制负数
		if (
			value.size() > 3
			&& value[0] == '-'
			&& value[1] == '0'
			&& (value[2] == 'x' || value[2] == 'X')
			)
		{
			char *endptr = nullptr;
			int64 v = strtoll(&value[3], &endptr, 16);
			js = -v;
			return true;
		}
		//单字符
		if (value.front() == '\'')
		{
			if (value.size() < 2)
			{
				Log("error para in ParseBaseValue. value str=%s", value.c_str());
				return false;
			}
			js = value[1];
			return true;
		}
		L_S_DEBUG("json parse : %s", value.c_str());
		js = json::parse(value);
		if (js.is_null())
		{
			Log("error para in ParseBaseValue. value str=%s", value.c_str());
			return false;
		}
		return true;
	}
	catch (exception& e)
	{
		Log("parse json fail. exception %s", e.what());
	}
	catch (...)
	{
		Log("parse json fail");
	}

	Log("error, unknow value when ParseBaseValue. ");
	return false;
}

bool SimpleCfg::IsBaseValue(const string &value)
{
	if (value.empty())
	{
		Log("error para in IsBaseValue");
		return true;
	}
	if (value[0] == '{' || value[0] == '[')
	{
		return false;
	}
	return true;
}

bool SimpleCfg::FindNameStr(CharIter &cur, const CharIter end, std::string &name)
{
	if (cur == end)
	{
		Log("error, FindNameStr input empty string");
		return false;
	}
	name.clear();

	//const char *cur_p = nullptr; //调试用
	//找开始，cur指向成员名开始位置。
	{
		//cur_p = &*cur;

		CharIter mem_name_start = cur;
		bool isFind = false;
		for (uint32 n = 0; n < 50000; ++n)
		{
			//cur 跳过注释 "//"
			CheckAndJumpComment(cur, end);
			if (cur == end)
			{
				//Log("no member found. n=%d", n);
				return false;
			}
			//匿名成员非法
			if (*cur == '{'
				)
			{
				Log("anonymous member name is error. mem_name_start= %s", &*mem_name_start);
				std::exception e;
				throw e;
			}
			if (
				(*cur >= 'a' && *cur <= 'z')
				||
				(*cur >= 'A' && *cur <= 'Z')
				)
			{
				isFind = true;
				break;
			}

			++cur;
			if (cur == end)
			{
				//	Log("找开始，cur指向成员名开始位置失败。 %s", cur_p);
				return false;
			}
		}
		if (!isFind)
		{
			//Log("找开始，cur指向成员名开始位置失败。 %s", cur_p);
			return false;
		}
	}

	//找名字结束
	{
		CharIter mem_name_start = cur;
		for (uint32 n = 0; n < 50000; ++n)
		{
			if (n >= 50000 - 1)
			{
				Log("error, FindNameStr endless loop ");
				return false;
			}
			if (cur == end)
			{
				Log("error, 找名字结束失败。 ");
				return false;
			}
			if (
				' ' == *cur
				|| '=' == *cur
				|| ':' == *cur
				|| '	' == *cur
				)
			{
				name.assign(mem_name_start, cur);
				return true;
			}
			++cur;
		}
	}
	return false;
}

bool SimpleCfg::Find1stMemStr(CharIter &cur, const CharIter end , string &name, string &value, bool &isDynamic)
{
	if (cur == end)
	{
		Log("error, FindMemStr input empty string");
		return false;
	}
	name.clear();
	value.clear();
	isDynamic = false;
	const char *cur_p = nullptr; //调试用



	//找名字结束
	cur_p = &*cur;
	if (!FindNameStr(cur, end, name))
	{
		Log("find member name fail %s", cur_p);
		return false;
	}
	
	if (DynamicStr == name)
	{
		L_S_DEBUG("is dynamic name");
		isDynamic = true;
		name.clear();
		if (!FindNameStr(cur, end, name))
		{
			Log("find member name fail %s", cur_p);
			return false;
		}
	}

	//跳过赋值符，空格cur指向成员值开始位置。
	{
		for (uint32 n = 0; n < 50000; ++n)
		{
			if (cur == end)
			{
				Log("error, 查找成员值失败");
				return false;
			}

			if (
				' ' == *cur
				|| '=' == *cur
				|| ':' == *cur
				|| '	' == *cur
				)
			{
				++cur;
				continue;
			}
			else
			{
				break;
			}
		}
	}

	//找对象值
	return FindValueStr(cur, end, value);
}

bool SimpleCfg::Find1stElment(CharIter &cur, const CharIter end, std::string &value)
{
	if (cur == end)
	{
		Log("error, FindMemStr input empty string");
		return false;
	}
	value.clear();

	//cur 跳过注释 "//"
	{
		CheckAndJumpComment(cur, end);
		if (cur == end)
		{
			L_S_DEBUG("no member found. ");
			return false;
		}
	}

	//跳过分隔符
	CheckAndJumpSCS(cur, end);
	//找对象值
	if (cur == end)
	{
		L_S_DEBUG("FindValueStr end with comment");
		return false;
	}
	return FindValueStr(cur, end, value);
}

bool SimpleCfg::FindValueStr(CharIter &cur, const CharIter end, std::string &value)
{
	L_S_DEBUG("FindValueStr cur=%s", &*cur);
	if (cur == end)
	{
		Log("error para");
		return false;
	}
	CharIter value_start = cur;

	CheckAndJumpSCS(cur, end);
	//找对象值结束位置
	if (*cur == '{' || *cur == '[')
	{
		char end_c = 0;
		char start_c = 0;
		if (*cur == '{')
		{
			start_c = '{';
			end_c = '}';
		}
		else
		{
			start_c = '[';
			end_c = ']';
		}

		//找到结束符号
		uint32  start_symbol_num = 0;
		uint32  end_symbol_num = 0;
		auto IsEndSymbol = [&](const char c)
		{
			if (c == start_c)
			{
				start_symbol_num++;
			}
			else if (c == end_c)
			{
				end_symbol_num++;
			}

			if (start_symbol_num == end_symbol_num)
			{
				return true;
			}
			return false;
		};
		CharIter it = std::find_if(cur, end, IsEndSymbol);
		if (it == end)
		{
			Log("error, find {} or [] fail, is not match");
			return false;
		}
		value.assign(value_start, it + 1);
		L_S_DEBUG("it=%s", (const char *)&(*it));
		cur = it;
		cur++;
		L_S_DEBUG("cur=%s", (const char *)&(*cur));
		L_S_DEBUG("FindMemStr, find obj end value=%s, cur=%s", (const char *)&(*value_start), (const char *)&(*cur));
		return true;
	}
	else//找基本值结束位置
	{

		L_S_DEBUG("FindValueStr, after jump split. cur=%s", &*cur);
		char last_char = 0; //IsEndSymbol 上次查找字符
		bool is_comment_end=false;
		auto IsEndSymbol = [&](const char c)
		{
			if ('}' == c || ']' == c )
			{
				return true;
			}
			if ('/' == c && last_char=='/')//找到 "//" 两个字符
			{
				is_comment_end = true;
				return true;
			}
			last_char = c;
			return IsSplitChar(c);
		};
		CharIter it = std::find_if(cur, end, IsEndSymbol);
		if (it == end)
		{
			//没结束符号
			value.assign(value_start, it);
			cur = end;
			return true;
		}
		if (value_start == it)//处理 “]”字符串
		{
			return false;
		}
		if (is_comment_end)
		{
			it--;
		}
		value.assign(value_start, it);
		cur = it + 1;
		return true;
	}
}

bool SimpleCfg::IsSplitChar(char c)
{
	if (
		 ',' == c
		|| ':' == c
		|| ';' == c
		|| '	' == c
		|| '\n' == c
		)
	{
		return true;
	}
	return false;
}

bool SimpleCfg::IsOperatorChar(const char c)
{
	if (
		'+' == c
		|| '-' == c
		|| '*' == c
		|| '/' == c
		)
	{
		return true;
	}
	return false;
}

nlohmann::json SimpleCfg::OperatorBaseValue(const nlohmann::json &left, char opt, const nlohmann::json &right)
{
	if (!left.is_number() || !right.is_number())
	{
		Log("error, unknow oprand is not number");
		return json();
	}
	double num_left = left;
	double num_right = right;

	Log(" %.1f opt %.1f", num_left, num_right);
	switch (opt)
	{
	case '+':
		num_left += num_right;
		break;
	case '-':
		num_left -= num_right;
		break;
	case '*':
		num_left *= num_right;
		break;
	case '/':
		if (0 == num_right)
		{
			Log("error, divide zero");
			return json();
		}
		num_left /= num_right;
		break;
	default:
		Log("unknow opt char[%c]", opt);
		return json();
		break;
	}
	Log(" = %.1f", num_left);
	if (num_left - (int64)num_left < 0.0000001)
	{
		return (int64)num_left;
	}
	return num_left;
}

