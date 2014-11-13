#ifndef __DEBUG_MANAGE_H__
#define __DEBUG_MANAGE_H__
#include "untils.h"

#define rt_dprintf(type,message)               	\
do                                              \
{                                            		\
    if (debug_check(type) == RT_TRUE)						\
        rt_kprintf message;                     \
}                                               \
while(0)

rt_bool_t debug_check(rt_uint32_t flag);


#endif

