#include "db_con.h"


bool CfgMgr::Init()
{
	L_DEBUG("init cfg");
	L_COND_F(m_comCfg.LoadFile());
	return true;
}
