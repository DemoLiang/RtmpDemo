#include "jfg_https.h"
#include <stdio.h>
#include <string.h>
#include "cJSON.h"

#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#if defined(MBEDTLS_PLATFORM_C)
#include "mbedtls/platform.h"
#endif

#include "mbedtls/net.h"
#include "mbedtls/debug.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/error.h"
#include "mbedtls/certs.h"

#include "cl_debug.h"

//retval 0£ºsuccess
static int parse_https_response(char* buf, int buflen, char **rtmpurl, int *value)
{
	char *token, *tmp, *url, *p;
	cJSON *pJson, *pSub;
	unsigned int content_len = 0;
	unsigned int url_len = 0;
	int response_code = 0;
	int bstate = 0;
	int ret = -1;

	if(NULL == buf || buflen == 0 || NULL == rtmpurl ){
		return ret;
	}

	token = strtok_r(buf, "\r\n", &p);
	if(token == NULL){
		return ret;
	}

	response_code = strtol(token + 9, NULL, 10);
	if(200 != response_code){
		return ret;
	}

	token = strtok_r( NULL, "\r\n", &p);
	while( token != NULL )
	{
		tmp = strstr(token, "Content-Length:");
		if(NULL != tmp){
			content_len = strtol(token + 15, NULL, 10);
		}
		//get msg
		if(content_len > 0 && content_len == strlen(token) &&
			'{' == token[0])
		{
			bstate = 1;
			break;
		}

		token = strtok_r( NULL, "\r\n", &p);
	}

	//parse json
	if (bstate)
	{
		pJson = cJSON_Parse(token);
		if(NULL == pJson){
			return ret;
		}

		do 
		{
			pSub = cJSON_GetObjectItem(pJson, "ret");
			if(NULL == pSub){
				break;
			}
			*value = pSub->valueint;
			if(0 != *value){
				ret = 0;
				break;
			}

			pSub = cJSON_GetObjectItem(pJson, "url");
			if(NULL == pSub){
				break;
			}
			//pSub->valuestring;
			if(NULL == pSub->valuestring){
				break;
			}

			url_len = strlen(pSub->valuestring) + 1;
			url = (char*)malloc(url_len);
			memset(url, 0, url_len);
			memcpy(url, pSub->valuestring, url_len - 1);
			*rtmpurl = url;	

			ret = 0;
		} while (0);

		cJSON_Delete(pJson);
	}

	return ret;
}

//int main( void )
int jfg_https_get(char* server_name, char* service_key, char* vid, char* sn, 
				  char* account, char** rtmpUrl, int* value)
{
    int ret, len;
    mbedtls_net_context server_fd;
    unsigned int flags;
    unsigned char buf[1024];
    const char *pers = "ssl_client1";

    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_ssl_context ssl;
    mbedtls_ssl_config conf;
    mbedtls_x509_crt cacert;

    mbedtls_net_init( &server_fd );
    mbedtls_ssl_init( &ssl );
    mbedtls_ssl_config_init( &conf );
	mbedtls_ctr_drbg_init( &ctr_drbg );

    mbedtls_entropy_init( &entropy );
    if((ret = mbedtls_ctr_drbg_seed( &ctr_drbg, mbedtls_entropy_func, &entropy,
		(const unsigned char *) pers, strlen(pers))) != 0 )
    {
        DUMP("failed! mbedtls_ctr_drbg_seed returned %d", ret );
        goto exit;
    }

    /*
     * 1. Start the connection
     */
    if( ( ret = mbedtls_net_connect( &server_fd, server_name, ROBOT_SERVER_PORT,
		MBEDTLS_NET_PROTO_TCP ) ) != 0 )
    {
        DUMP("failed! mbedtls_net_connect returned %d", ret );
        goto exit;
	}

    /*
     * 2. Setup stuff
     */
    if((ret = mbedtls_ssl_config_defaults( &conf, MBEDTLS_SSL_IS_CLIENT, 
		MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT)) != 0)
    {
        DUMP("failed! mbedtls_ssl_config_defaults returned %d", ret );
        goto exit;
    }

    /* OPTIONAL is not optimal for security,
     * but makes interop easier in this simplified example */
    mbedtls_ssl_conf_authmode( &conf, MBEDTLS_SSL_VERIFY_OPTIONAL );
    mbedtls_ssl_conf_ca_chain( &conf, &cacert, NULL );
    mbedtls_ssl_conf_rng( &conf, mbedtls_ctr_drbg_random, &ctr_drbg );
//    mbedtls_ssl_conf_dbg( &conf, my_debug, stdout );

    if((ret = mbedtls_ssl_setup(&ssl, &conf)) != 0)
    {
        DUMP("failed! mbedtls_ssl_setup returned %d", ret);
        goto exit;
    }

    if((ret = mbedtls_ssl_set_hostname( &ssl, "mbed TLS Server 1")) != 0)
    {
        DUMP("failed! mbedtls_ssl_set_hostname returned %d", ret );
        goto exit;
    }

    mbedtls_ssl_set_bio(&ssl, &server_fd, mbedtls_net_send, mbedtls_net_recv, NULL);

    /*
     * 4. Handshake
     */
    while((ret = mbedtls_ssl_handshake(&ssl)) != 0)
    {
        if(ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
        {
            DUMP("failed! mbedtls_ssl_handshake returned -0x%x", -ret);
            goto exit;
        }
    }

	//"/oss/v1/get_upload_url?service_key=ooaH6CPNxib8MwTGtcTGGM3pYfc8Slhn&vid=000F&sn=200000000001&account=18927486726"
	len = 0;
	len += sprintf((char*)buf + len, "GET /oss/v1/get_upload_url?service_key=%s&vid=%s&sn=%s&account=%s HTTP/1.0\r\n",
		service_key, vid, sn, account);
	len += sprintf((char*)buf + len, "Host: %s:%s\r\n", server_name, ROBOT_SERVER_PORT);
	len += sprintf((char*)buf + len, "Connection: close\r\n");
	len += sprintf((char*)buf + len, "User-Agent: Jiafeigou/linux-1.0\r\n");
	len += sprintf((char*)buf + len, "\r\n");

    while((ret = mbedtls_ssl_write( &ssl, buf, len)) <= 0)
    {
        if( ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE )
        {
            DUMP("failed! mbedtls_ssl_write returned %d", ret);
            goto exit;
        }
    }

    len = ret;
    DUMP(" %d bytes written\n\n%s", len, (char *)buf);

    /*
     * 7. Read the HTTP response
     */
    do
    {
        len = sizeof( buf ) - 1;
        memset( buf, 0, sizeof( buf ) );
        ret = mbedtls_ssl_read( &ssl, buf, len );

        if(ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE)
            continue;

        if( ret == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY )
            break;

        if( ret < 0 )
        {
            DUMP("failed! mbedtls_ssl_read returned %d", ret );
            break;
        }

        if( ret == 0 )
        {
            DUMP( "\n\nEOF\n\n" );
            break;
        }

        len = ret;
        DUMP("%d bytes read %s", len, (char *) buf );

		//parse http response
		if( (ret = parse_https_response((char*)buf, len, rtmpUrl, value) ) == 0 )
		{
			break;
		}
    }
    while( 1 );
    mbedtls_ssl_close_notify( &ssl );

exit:

    mbedtls_net_free( &server_fd );
    mbedtls_ssl_free( &ssl );
    mbedtls_ssl_config_free( &conf );
    mbedtls_ctr_drbg_free( &ctr_drbg );
    mbedtls_entropy_free( &entropy );

    return( ret );
}
