#ifndef __NETPHYSICS_H__
#define __NETPHYSICS_H__
#include "rtthread.h"
#include "netprotocol.h"

rt_err_t netprotocol_connect_status(void);

#ifndef NPDU_THREAD_PRI_IS 
#define NPDU_THREAD_PRI_IS		RT_THREAD_PRIORITY_MAX/2+2
#endif

#endif


