/*********************************************************
 * file : cl_debug.h
 * auth : liangguiming
 * date : 2016-10-09
 * desc : this file is define the debug printf for ssh login
 * ******************************************************/

#ifndef __CL_DEBUG_H__
#define __CL_DEBUG_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdarg.h>

//#define __CL_DEBUG_ENABLE__
#ifdef __CL_DEBUG_ENABLE__

#define CL_DEBUG_FILE "/dev/console"

#define CL_PRINTF(fmt, arg...) do{ \
	FILE *fp = NULL; \
	if(access(CL_DEBUG_FILE,F_OK) == 0){\
	fp = fopen(CL_DEBUG_FILE,"w+");	\
	if(fp != NULL){	\
		fprintf(fp,"[%s-%s-%d] "fmt,__FILE__,__FUNCTION__,__LINE__,##arg);	\
	\
	fclose(fp);	}}\
}while(0)

#define DUMP CL_PRINTF
#else

#define CL_PRINTF(fmt, arg...)
#define DUMP CL_PRINTF

#endif
#endif
