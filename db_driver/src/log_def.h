/*
	日志宏
*/
#pragma once
#include <string>
#include "svr_util/include/log_file.h"

#define LOG_NAMESPACE su::  

#define _LOG_PRINTER_(x, log_lv, ...) LOG_NAMESPACE LogMgr::Ins().Printf(log_lv, __FILE__, __LINE__, __FUNCTION__, x, ##__VA_ARGS__);
#define _LOG_COND_PRINTER_(cond, ...) LOG_NAMESPACE LogMgr::Ins().PrintfCond(LOG_NAMESPACE LL_ERROR, __FILE__, __LINE__, __FUNCTION__, cond, ##__VA_ARGS__); 

#define L_FATAL(x, ...)  _LOG_PRINTER_(x, LOG_NAMESPACE LL_FATAL, ##__VA_ARGS__);
#define L_ERROR(x, ...)  _LOG_PRINTER_(x, LOG_NAMESPACE LL_ERROR, ##__VA_ARGS__);
#define L_WARN(x, ...)   _LOG_PRINTER_(x, LOG_NAMESPACE LL_WARN, ##__VA_ARGS__);
#define L_INFO(x, ...)   _LOG_PRINTER_(x, LOG_NAMESPACE LL_INFO, ##__VA_ARGS__);
#define L_DEBUG(x, ...)  _LOG_PRINTER_(x, LOG_NAMESPACE LL_DEBUG, ##__VA_ARGS__);
#define L_TRACE(x, ...)  _LOG_PRINTER_(x, LOG_NAMESPACE LL_TRACE, ##__VA_ARGS__);


//日志记录宏，每秒一次
#define L_TRACE_SEC(iSec, x, ...)\
    do\
    {\
        int iCurrTime = time(0);\
        static int iBaseLogTime = 0;\
        if (iCurrTime >= iBaseLogTime + iSec)\
        {\
            iBaseLogTime = iCurrTime;\
            _LOG_PRINTER_(x, LOG_NAMESPACE LL_TRACE, ##__VA_ARGS__);\
        }\
    }while(0);

//简化if语句写法,让代码简洁，一但习惯，提高编写效率和浏览代码效率。
#define L_COND(cond, ret, ...)\
	do{\
	if(!(cond)){\
	_LOG_COND_PRINTER_("condition fail \[" #cond "]. ", ##__VA_ARGS__); \
	return ret;\
	}	\
	}while(0);

#define L_COND_F(cond, args...)	L_COND(cond, false, ##args)
#define L_COND_V(cond, args...)	L_COND(cond, void(), ##args)

#define COND(cond, ret)\
	do{\
	if(!(cond)){\
	return ret;\
	}	\
	}while(0);

#define COND_F(cond) COND(cond, false)
#define COND_V(cond) COND(cond, void())

#define L_ASSERT(cond)\
	do{\
	if(!(cond)){\
	L_ERROR("assert error, condition \[" #cond "]. "); \
	*(int *)(nullptr) = 1;\
	}	\
	}while(0);



