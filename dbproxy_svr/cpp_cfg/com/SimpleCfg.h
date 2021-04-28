/*
从字符串解析出配置信息
存为json对象
分析非法格式（比如数组元素类型不一致）
*/

#pragma once
#include "json.hpp"


//#define PRINT_DEBUG_LOG  //打印调试日志
#ifdef PRINT_DEBUG_LOG
#define L_S_DEBUG(x, ...)   Log(x, ##__VA_ARGS__)
#else
#define L_S_DEBUG(x, ...)  
#endif
namespace SimpleCfgLog
{
	//打印日志
	void Log(const char * pattern, ...);
	void EnableLog(bool enable_log);
}




using CharIter = std::string::const_iterator;


//解析简单配置格式
//js["dynamic"]=["a","b"] //用来存放本对象动态变量名
//数组元素不能有dynamic关键字
class SimpleCfg : public nlohmann::json
{
public:
	//文本文件json对象
	bool ParseFile(const std::string &cfg);
	//字符串解析成json对象
	bool ParseStr(const std::string &cfg);
	const char *c_str();
	static const char *DynamicStr;
	//return true表示合法的cfg对象
	bool IsEnableObj(const nlohmann::json &js);

private:
	bool IsEnableArray(const nlohmann::json &js);
	//字符串解析成json对象
	//@start, end 字符串迭代器，错误用户自己负责, 输入为{..}或者去掉{}
	bool ParseObj(CharIter start, CharIter end, nlohmann::json &js);
	//字符串解析成json数组
	//@start, end 字符串迭代器，错误用户自己负责， 输入为[....]
	bool ParseArray(CharIter start, CharIter end, nlohmann::json &js);


	//检查第一个字符，发现可能是注释就跳过注释位置
	//@cur [in/out] 
	//		输入 检查开始位置。
	//		输出 如果开始位置的字符串是是注释，跳过注释，指向正常语法字符。 最后一行注释，就跳到字符串尾端 string::end();
	//return true表示操作了跳过
	bool CheckAndJumpComment(CharIter &cur, CharIter end);
	bool CheckAndJumpSplit(CharIter &cur, CharIter end);
	bool CheckAndJumpSpace(CharIter &cur, CharIter end);

	//跳过 split comment space
	//
	void CheckAndJumpSCS(CharIter &cur, CharIter end);

	//解析基础值，
	//@js [out] 值赋值给js["base"]=xx
	bool ParseBaseValue(const std::string &value, nlohmann::json &js);

	bool IsBaseValue(const std::string &value);

	//@fun 查找第一个成员字符串
	//比如：
	//输入  n={a=3,b}， 输出name, value 为 n {a=3,b}
	//输入  a=3,b=3， 输出name, value 为  a 3
	//输入  n=[..]， 输出name, value 为  n [..]
	//@start cur [in/out] 输入，字符串开始位置。输出 下一个成员字符串开始位置
	//@name [out] 成员字符名
	//@value [out] 成员值
	//@isDynamic [out] true表示是动态成员
	//return 找不到成员返回false.
	bool Find1stMemStr(CharIter &cur, const CharIter end, std::string &name, std::string &value, bool &isDynamic);

	bool FindNameStr(CharIter &cur, const CharIter end, std::string &name);

	//查找第一个元素字符串
	//比如：
	//输入  [3,4]， 输出value 为 3
	//输入  ["abc",4]， 输出value 为 abc
	//@start cur [in/out] 输入，字符串开始位置。输出 下一个成员字符串开始位置
	//@end [in] 数组字符串结束位置
	//@value [out] 成员值
	//return 找不到成员返回false.
	bool Find1stElment(CharIter &cur, const CharIter end, std::string &value);

	//查找值字符串
	//@start cur [in/out] 输入，值字符串开始位置。输出 值字符串结尾+1位置
	//@end [in] 有效字符串结束位置
	//@value [out] 成员值
	//return 找不到成员返回false.
	bool FindValueStr(CharIter &cur, const CharIter end, std::string &value);

	bool IsSplitChar(char c);
	//有运算符
	static bool IsOperatorChar(const char c);

	nlohmann::json OperatorBaseValue(const nlohmann::json &left, char opt, const nlohmann::json &right);
};

