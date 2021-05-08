/*
brief: use example and test code
*/
#include "../com/unit_test.h"
#include <functional>
#include <limits>
#include <queue>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include "function_traits.h"
#include "db_driver/include/db_driver.h"

using namespace std;
using namespace lc;
using namespace su;
using namespace db;
using namespace proto;


//通过函数名，获取函数参数，返回值 类型 。
//只能适用 1-2个参数的函数。
namespace
{
	void f1(int a, uint64_t b)
	{

	}	
	void f2(uint8_t a, uint64_t b)
	{

	}
	int f3(uint8_t a)
	{
		return 0;
	}
	template<class Fun>
	void RegProtoParse3(Fun f)
	{
		//typename std::function<Fun>::argument_type t2 = 231; //ok
		typename std::function<Fun>::first_argument_type t2 = 231; //ok
		typename std::function<Fun>::second_argument_type t1 = 231; //ok
		typename std::function<Fun>::result_type *p=nullptr;

		//UNIT_INFO("t2 %d %d", t2, sizeof(t2))
		//UNIT_INFO("para3");
	}
}

UNITTEST(fun3)
{
	//RegProtoParse3<decltype(f2)>(f2);
	RegProtoParse3<decltype(f1)>(f1);
	//RegProtoParse3<decltype(f3)>(f3);
}