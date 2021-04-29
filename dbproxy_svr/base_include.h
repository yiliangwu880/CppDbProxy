
#pragma once
#include <string>
#include <vector>
#include <map>
#include "libevent_cpp/include/include_all.h"
#include "log_def.h"
#include "svr_util/include/singleton.h"
#include "svr_util/include/read_cfg.h"
#include "cfg.h"
#include "svr_util/include/typedef.h"
#include "proto/dbTableDef.h"

static const int MAX_SQL_STR_SIZE = 1024 * 1024 * 4;