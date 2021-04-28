/*
 管理连接db客户端
*/

#pragma once
#include "base_include.h"
#include "cpp_cfg/CC_Cfg.h"

class CfgMgr : public Singleton<CfgMgr>
{
public:
	bool Init();
	const Cfg &GetCfg() const { return m_cfg; }

private:
	Cfg m_cfg;
};