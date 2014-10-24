#ifndef __FPRINTHANDLE_H__
#define __FPRINTHANDLE_H__
#include "fprint.h"

#define FP_EVNT_NORMAL_MODE					(1<<0)
#define FP_EVNT_REGISTER_MODE				(1<<1)
#define FP_EVNT_ALL									(1<<2)

rt_err_t fp_event_process(rt_uint8_t mode,rt_uint32_t type);

#endif

