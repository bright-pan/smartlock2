#ifndef __CAMERA_H__
#define __CAMERA_H__A

#include "rtthread.h"
#include "rtdevice.h"
#include <dfs_init.h>
#include <dfs_elm.h>
#include <dfs_fs.h>
#include "dfs_posix.h"
#include "alarm.h"

#define CM_MAKE_PIC_NAME         "/1.jpg"


//camera mail
typedef struct 
{
	ALARM_TYPEDEF AlarmType;
}CameraMail,*CameraMail_p;




#endif


