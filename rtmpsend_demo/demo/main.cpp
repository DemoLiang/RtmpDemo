#include <stdio.h>
#include "librtmp_send264.h"
#include "jfg_https.h"
#include "cl_debug.h"

int main(int argc, char* argv[])
{

	int ret = RTMP264_ERROR;
	char *url = NULL;
	int value = -1;

	ret = RTMP264_Init();
	if(ret == RTMP264_FAIL)
	{
		CL_PRINTF("RTMP_Init fail!\n");
		return RTMP264_ERROR;
	}

	//ret = RTMP264_GetUrl(url,sizeof(url));
	ret = jfg_https_get((char*)ROBOT_SERVER_NAME, (char*)ROBOT_SERVICE_KEY, (char*)VID, (char*)SN, (char*)ACCOUNT, &url, &value);
	if(ret == RTMP264_ERROR)
	{
		CL_PRINTF("get url fail!\n");
		return RTMP264_ERROR;
	}

	ret = RTMP264_Connect(url);
	if(ret == RTMP264_ERROR)
	{
		CL_PRINTF("connect fail!\n");
		return RTMP264_ERROR;
	}
	
	RTMP264_Send(read_buffer_callback);

	RTMP264_Close();

	return 0;
}

