#ifndef _JFG_HTTP_H 
#define _JFG_HTTP_H

#define ROBOT_SERVER_PORT "443"
#define ROBOT_SERVER_NAME "www.robotscloud.com"
#define ROBOT_SERVICE_KEY "service key"
#define VID         "8888"
#define SN          "000000000001"
#define ACCOUNT     "18888888888"

//test
#define ROBOT_SERVER_NAME_LEN  64
#define SERVICE_KEY_LEN        64
#define VID_LEN                10
#define SN_LEN                 32
#define ACCOUNT_LEN            32
typedef struct _PUSHFLOW_INFO
{
	char szServerName[ROBOT_SERVER_NAME_LEN];
	char szServiceKey[SERVICE_KEY_LEN];
	char szVid[VID_LEN];
	char szSn[SN_LEN];
	char szAccount[ACCOUNT_LEN];
	int nTimes;
}PUSHFLOW_INFO;

/**
* @fn jfg_https_get
* @brief 
* @param char* service_key(in), 
* @param char* vid(in),
* @param char* sn(in),
* @param char* account(in),
* @param char** rtmpUrl(out),
* @param int*  ret(out), 
* @retval 0:only get msg success£» need to judge by ret, value equal 0 success, 
*           need free rtmpUrl; value equal other value, no need free rtmpUrl;
**/
int jfg_https_get(char* server_name, char* service_key, char* vid, char* sn, 
				  char* account, char** rtmpUrl, int *value);

#endif
