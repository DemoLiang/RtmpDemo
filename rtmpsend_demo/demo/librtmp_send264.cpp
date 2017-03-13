#include <stdio.h>
#include <stdlib.h>
#include "librtmp_send264.h"
#include "rtmp.h"   
#include "rtmp_sys.h"   
#include "amf.h"  
#include "sps_decode.h"
#include "cl_debug.h"

#ifdef WIN32     
#include <windows.h>  
#pragma comment(lib,"WS2_32.lib")   
#pragma comment(lib,"winmm.lib")  
#endif 

unsigned int  m_nFileBufSize; 
unsigned int  nalhead_pos;
RTMP* m_pRtmp;  
RTMPMetadata metaData;
unsigned char *m_pFileBuf;  
unsigned char *m_pFileBuf_tmp;
unsigned char* m_pFileBuf_tmp_old;	//used for realloc
FILE *fp_send1;

char *RTMP264_Malloc(int size)
{
	char *ptr = NULL;
	ptr = (char *)malloc(size);
	if(ptr == NULL)
	{
		CL_PRINTF("malloc fail!\n");
		return NULL;
	}
	memset(ptr,0x00,size);
	return ptr;
}

int InitSockets()    
{    
	#ifdef WIN32     
		WORD version;    
		WSADATA wsaData;    
		version = MAKEWORD(1, 1);    
		return (WSAStartup(version, &wsaData) == 0);    
	#else     
		return TRUE;    
	#endif     
}

inline void CleanupSockets()    
{    
	#ifdef WIN32     
		WSACleanup();    
	#endif     
}    

char *put_byte( char *output, uint8_t nVal )    
{    
	output[0] = nVal;    
	return output+1;    
}   

char *put_be16(char *output, uint16_t nVal )    
{    
	output[1] = nVal & 0xff;    
	output[0] = nVal >> 8;    
	return output+2;    
}  

char *put_be24(char *output,uint32_t nVal )    
{    
	output[2] = nVal & 0xff;    
	output[1] = nVal >> 8;    
	output[0] = nVal >> 16;    
	return output+3;    
}    
char *put_be32(char *output, uint32_t nVal )    
{    
	output[3] = nVal & 0xff;    
	output[2] = nVal >> 8;    
	output[1] = nVal >> 16;    
	output[0] = nVal >> 24;    
	return output+4;    
}    
char *put_be64( char *output, uint64_t nVal )    
{    
	output=put_be32( output, nVal >> 32 );    
	output=put_be32( output, nVal );    
	return output;    
}  

char *put_amf_string( char *c, const char *str )    
{    
	uint16_t len = strlen( str );    
	c=put_be16( c, len );    
	memcpy(c,str,len);    
	return c+len;    
}    
char *put_amf_double( char *c, double d )    
{    
	*c++ = AMF_NUMBER;  /* type: Number */    
	{    
		unsigned char *ci, *co;    
		ci = (unsigned char *)&d;    
		co = (unsigned char *)c;    
		co[0] = ci[7];    
		co[1] = ci[6];    
		co[2] = ci[5];    
		co[3] = ci[4];    
		co[4] = ci[3];    
		co[5] = ci[2];    
		co[6] = ci[1];    
		co[7] = ci[0];    
	}    
	return c+8;    
}  

int read_buffer_callback(unsigned char *buf, int buf_size )
{
	int true_size = 0;
	if(buf == NULL || buf_size == 0)
	{
		CL_PRINTF("buf is zero!\n");
		return RTMP264_ERROR;
	}

	if(!feof(fp_send1))
	{
		true_size=fread(buf,1,buf_size,fp_send1);
	}
	else
	{
		//maby we can get the buf from camera
		return RTMP264_ERROR;
	}

	return true_size;
}

int RTMP264_Init()
{
	nalhead_pos = 0;
	m_nFileBufSize = BUFFER_SIZE;

	m_pFileBuf = (unsigned char *)malloc(BUFFER_SIZE);
	if(m_pFileBuf == NULL)
	{
		CL_PRINTF("malloc m_pFileBuf fail!\n");
		return RTMP264_ERROR;
	}
	m_pFileBuf_tmp = (unsigned char *)malloc(BUFFER_SIZE);
	if(m_pFileBuf_tmp == NULL)
	{
		CL_PRINTF("malloc m_pFileBuf_tmp fail!\n");
		goto error;
	}

	fp_send1 = fopen("video.h264","rb");
	if(fp_send1 == NULL)
	{
		CL_PRINTF("open video file fail!\n");
		return RTMP264_ERROR;
	}
	
	return RTMP264_OK;
error:
	if(m_pFileBuf)
		free(m_pFileBuf);
	if(m_pFileBuf_tmp)
		free(m_pFileBuf_tmp);
	return RTMP264_ERROR;
}
int RTMP264_GetUrl(char *url,int len)
{
	FILE *fp = NULL;
	int ret = RTMP264_ERROR;

	if(url == NULL || len <= 0)
	{
		CL_PRINTF("url is null!\n");
		return RTMP264_ERROR;
	}
	if((fp = fopen("url","rb")) == NULL)
	{
		CL_PRINTF("open the url file fail!\n");
		return RTMP264_ERROR;
	}
	ret = fread(url,1,len,fp);
	if(ret < 0)
	{
		CL_PRINTF("read url fail!\n");
		goto end;
	}
	url[strlen(url)-1] = '\0';
	fclose(fp);
	return ret;
end:
	fclose(fp);
}


int RTMP264_Connect(const char* url)  
{  
	if(url == NULL) {
		CL_PRINTF("connect url is null!\n");
		return RTMP264_ERROR;
	}

	InitSockets();  

	m_pRtmp = RTMP_Alloc();
	if(m_pRtmp == NULL) {
		CL_PRINTF("alloc rtmp fail!\n");
		return RTMP264_ERROR;
	}
	RTMP_Init(m_pRtmp);
	
	if (RTMP_SetupURL(m_pRtmp,(char*)url) == FALSE)
	{
		RTMP_Free(m_pRtmp);
		return RTMP264_ERROR;
	}
	RTMP_EnableWrite(m_pRtmp);
	if (RTMP_Connect(m_pRtmp, NULL) == FALSE) 
	{
		RTMP_Free(m_pRtmp);
		return RTMP264_ERROR;
	} 

	if (RTMP_ConnectStream(m_pRtmp,0) == FALSE)
	{
		RTMP_Close(m_pRtmp);
		RTMP_Free(m_pRtmp);
		return RTMP264_ERROR;
	}
	return true;  
}  


void RTMP264_Close()  
{  
	if(m_pRtmp)  
	{  
		RTMP_Close(m_pRtmp);  
		RTMP_Free(m_pRtmp);  
		m_pRtmp = NULL;  
	}  
	CleanupSockets();   
	if (m_pFileBuf != NULL)
	{  
		free(m_pFileBuf);
	}  
	if (m_pFileBuf_tmp != NULL)
	{  
		free(m_pFileBuf_tmp);
	}
} 

int SendPacket(unsigned int nPacketType,unsigned char *data,unsigned int size,unsigned int nTimestamp)  
{  
	int nRet =0;
	RTMPPacket *packet;
	if(data == NULL || size <= 0)
	{
		CL_PRINTF("send data is zero!\n");
		return RTMP264_ERROR;
	}

	packet = (RTMPPacket *)malloc(RTMP_HEAD_SIZE+size);
	if(packet == NULL)
	{
		CL_PRINTF("malloc packet fail!\n");
		return RTMP264_ERROR;
	}
	memset(packet,0,RTMP_HEAD_SIZE);
	
	packet->m_body = (char *)packet + RTMP_HEAD_SIZE;
	packet->m_nBodySize = size;
	memcpy(packet->m_body,data,size);
	packet->m_hasAbsTimestamp = 0;
	packet->m_packetType = nPacketType;
	packet->m_nInfoField2 = m_pRtmp->m_stream_id;
	packet->m_nChannel = 0x04;

	packet->m_headerType = RTMP_PACKET_SIZE_LARGE;
	if (RTMP_PACKET_TYPE_AUDIO ==nPacketType && size !=4)
	{
		packet->m_headerType = RTMP_PACKET_SIZE_MEDIUM;
	}
	packet->m_nTimeStamp = nTimestamp;

	if (RTMP_IsConnected(m_pRtmp))
	{
		nRet = RTMP_SendPacket(m_pRtmp,packet,TRUE);
	}
	free(packet);
	return nRet;  
}  

int SendVideoSpsPps(unsigned char *pps,int pps_len,unsigned char *sps,int sps_len)
{
	RTMPPacket *packet=NULL;
	unsigned char *body=NULL;
	int i;
	int nRet = 0;
	if(pps == NULL || sps == NULL || pps_len <= 0 || sps_len <= 0)
	{
		CL_PRINTF("send video sps pps is zero!\n");
		return RTMP264_ERROR;
	}

	packet = (RTMPPacket *)malloc(RTMP_HEAD_SIZE+1024);
	if(packet == NULL)
	{
		CL_PRINTF("malloc packet fail!\n");
		return RTMP264_ERROR;
	}
	memset(packet,0,RTMP_HEAD_SIZE+1024);

	packet->m_body = (char *)packet + RTMP_HEAD_SIZE;
	body = (unsigned char *)packet->m_body;
	i = 0;
	body[i++] = 0x17;
	body[i++] = 0x00;
	body[i++] = 0x00;
	body[i++] = 0x00;
	body[i++] = 0x00;

	/*AVCDecoderConfigurationRecord*/
	body[i++] = 0x01;
	body[i++] = sps[1];
	body[i++] = sps[2];
	body[i++] = sps[3];
	body[i++] = 0xff;

	/*sps*/
	body[i++]   = 0xe1;
	body[i++] = (sps_len >> 8) & 0xff;
	body[i++] = sps_len & 0xff;
	memcpy(&body[i],sps,sps_len);
	i +=  sps_len;

	/*pps*/
	body[i++]   = 0x01;
	body[i++] = (pps_len >> 8) & 0xff;
	body[i++] = (pps_len) & 0xff;
	memcpy(&body[i],pps,pps_len);
	i +=  pps_len;

	packet->m_packetType = RTMP_PACKET_TYPE_VIDEO;
	packet->m_nBodySize = i;
	packet->m_nChannel = 0x04;
	packet->m_nTimeStamp = 0;
	packet->m_hasAbsTimestamp = 0;
	packet->m_headerType = RTMP_PACKET_SIZE_MEDIUM;
	packet->m_nInfoField2 = m_pRtmp->m_stream_id;

	nRet = RTMP_SendPacket(m_pRtmp,packet,TRUE);
	free(packet);
	return nRet;
}

int SendH264Packet(unsigned char *data,unsigned int size,int bIsKeyFrame,unsigned int nTimeStamp)  
{  
	unsigned char *body = NULL; 
	int i = 0; 

	if(data == NULL && size<11){  
		CL_PRINTF("data is NULL and size less!\n");
		return RTMP264_ERROR;  
	}  
	body= (unsigned char*)malloc(size+9);  
	if(body == NULL)
	{
		CL_PRINTF("malloc fail\n");
		return -1;
	}
	memset(body,0,size+9);

	if(bIsKeyFrame){  
		body[i++] = 0x17;// 1:Iframe  7:AVC   
		body[i++] = 0x01;// AVC NALU   
		body[i++] = 0x00;  
		body[i++] = 0x00;  
		body[i++] = 0x00;  


		// NALU size   
		body[i++] = size>>24 &0xff;  
		body[i++] = size>>16 &0xff;  
		body[i++] = size>>8 &0xff;  
		body[i++] = size&0xff;
		// NALU data   
		
		memcpy(&body[i],data,size);  
		
		SendVideoSpsPps(metaData.Pps,metaData.nPpsLen,metaData.Sps,metaData.nSpsLen);
	}else{  
		body[i++] = 0x27;// 2:Pframe  7:AVC   
		body[i++] = 0x01;// AVC NALU   
		body[i++] = 0x00;  
		body[i++] = 0x00;  
		body[i++] = 0x00;  


		// NALU size   
		body[i++] = size>>24 &0xff;  
		body[i++] = size>>16 &0xff;  
		body[i++] = size>>8 &0xff;  
		body[i++] = size&0xff;
		// NALU data   
		memcpy(&body[i],data,size);  
	}  
	

	int bRet = SendPacket(RTMP_PACKET_TYPE_VIDEO,body,i+size,nTimeStamp);  

	free(body);  

	return bRet;  
} 

int ReadFirstNaluFromBuf(NaluUnit &nalu,int (*read_buffer)(uint8_t *buf, int buf_size)) 
{
	int naltail_pos=nalhead_pos;
	memset(m_pFileBuf_tmp,0,BUFFER_SIZE);
	
	while(nalhead_pos<m_nFileBufSize)  
	{  
		//search for nal header
		if(m_pFileBuf[nalhead_pos++] == 0x00 && 
			m_pFileBuf[nalhead_pos++] == 0x00) 
		{
			if(m_pFileBuf[nalhead_pos++] == 0x01)
				goto gotnal_head;
			else 
			{
				//cuz we have done an i++ before,so we need to roll back now
				nalhead_pos--;		
				if(m_pFileBuf[nalhead_pos++] == 0x00 && 
					m_pFileBuf[nalhead_pos++] == 0x01)
					goto gotnal_head;
				else
					continue;
			}
		}
		else 
			continue;

		//search for nal tail which is also the head of next nal
gotnal_head:
		//normal case:the whole nal is in this m_pFileBuf
		naltail_pos = nalhead_pos;  
		while (naltail_pos<m_nFileBufSize)  
		{  
			if(m_pFileBuf[naltail_pos++] == 0x00 && 
				m_pFileBuf[naltail_pos++] == 0x00 )
			{  
				if(m_pFileBuf[naltail_pos++] == 0x01)
				{
					nalu.size = (naltail_pos-3)-nalhead_pos;
					break;
				}
				else
				{
					naltail_pos--;
					if(m_pFileBuf[naltail_pos++] == 0x00 &&
						m_pFileBuf[naltail_pos++] == 0x01)
					{	
						nalu.size = (naltail_pos-4)-nalhead_pos;
						break;
					}
				}
			}  
		}

		nalu.type = m_pFileBuf[nalhead_pos]&0x1f; 
		memcpy(m_pFileBuf_tmp,m_pFileBuf+nalhead_pos,nalu.size);
		nalu.data=m_pFileBuf_tmp;
		nalhead_pos=naltail_pos;
		return TRUE;   		
	}
}

int ReadOneNaluFromBuf(NaluUnit &nalu,int (*read_buffer)(uint8_t *buf, int buf_size))  
{    
	
	int naltail_pos=nalhead_pos;
	int ret;
	int nalustart;
	memset(m_pFileBuf_tmp,0,BUFFER_SIZE);
	nalu.size=0;
	while(1)
	{
		if(nalhead_pos==NO_MORE_BUFFER_TO_READ)
			return FALSE;
		while(naltail_pos<m_nFileBufSize)  
		{  
			//search for nal tail
			if(m_pFileBuf[naltail_pos++] == 0x00 && 
				m_pFileBuf[naltail_pos++] == 0x00) 
			{
				if(m_pFileBuf[naltail_pos++] == 0x01)
				{	
					nalustart=3;
					goto gotnal ;
				}
				else 
				{
					//cuz we have done an i++ before,so we need to roll back now
					naltail_pos--;		
					if(m_pFileBuf[naltail_pos++] == 0x00 && 
						m_pFileBuf[naltail_pos++] == 0x01)
					{
						nalustart=4;
						goto gotnal;
					}
					else
						continue;
				}
			}
			else 
				continue;

			gotnal:	
 				/**
				 *special case1:parts of the nal lies in a m_pFileBuf and we have to read from buffer 
				 *again to get the rest part of this nal
				 */
				if(nalhead_pos==GOT_A_NAL_CROSS_BUFFER || nalhead_pos==GOT_A_NAL_INCLUDE_A_BUFFER)
				{
					nalu.size = nalu.size+naltail_pos-nalustart;
					if(nalu.size>BUFFER_SIZE)
					{
						m_pFileBuf_tmp_old=m_pFileBuf_tmp;	//// save pointer in case realloc fails
						if((m_pFileBuf_tmp = (unsigned char*)realloc(m_pFileBuf_tmp,nalu.size)) ==  NULL )
						{
							free( m_pFileBuf_tmp_old );  // free original block
							return FALSE;
						}
					}
					memcpy(m_pFileBuf_tmp+nalu.size+nalustart-naltail_pos,m_pFileBuf,naltail_pos-nalustart);
					nalu.data=m_pFileBuf_tmp;
					nalhead_pos=naltail_pos;
					return TRUE;
				}
				//normal case:the whole nal is in this m_pFileBuf
				else 
				{  
					nalu.type = m_pFileBuf[nalhead_pos]&0x1f; 
					nalu.size=naltail_pos-nalhead_pos-nalustart;
					if(nalu.type==0x06)
					{
						nalhead_pos=naltail_pos;
						continue;
					}
					memcpy(m_pFileBuf_tmp,m_pFileBuf+nalhead_pos,nalu.size);
					nalu.data=m_pFileBuf_tmp;
					nalhead_pos=naltail_pos;
					return TRUE;    
				} 					
		}

		if(naltail_pos>=m_nFileBufSize && nalhead_pos!=GOT_A_NAL_CROSS_BUFFER && nalhead_pos != GOT_A_NAL_INCLUDE_A_BUFFER)
		{
			nalu.size = BUFFER_SIZE-nalhead_pos;
			nalu.type = m_pFileBuf[nalhead_pos]&0x1f; 
			memcpy(m_pFileBuf_tmp,m_pFileBuf+nalhead_pos,nalu.size);
			if((ret=read_buffer(m_pFileBuf,m_nFileBufSize))<BUFFER_SIZE)
			{
				memcpy(m_pFileBuf_tmp+nalu.size,m_pFileBuf,ret);
				nalu.size=nalu.size+ret;
				nalu.data=m_pFileBuf_tmp;
				nalhead_pos=NO_MORE_BUFFER_TO_READ;
				return FALSE;
			}
			naltail_pos=0;
			nalhead_pos=GOT_A_NAL_CROSS_BUFFER;
			continue;
		}
		if(nalhead_pos==GOT_A_NAL_CROSS_BUFFER || nalhead_pos == GOT_A_NAL_INCLUDE_A_BUFFER)
		{
			nalu.size = BUFFER_SIZE+nalu.size;
				
				m_pFileBuf_tmp_old=m_pFileBuf_tmp;	//// save pointer in case realloc fails
				if((m_pFileBuf_tmp = (unsigned char*)realloc(m_pFileBuf_tmp,nalu.size)) ==  NULL )
				{
					free( m_pFileBuf_tmp_old );  // free original block
					return FALSE;
				}

			memcpy(m_pFileBuf_tmp+nalu.size-BUFFER_SIZE,m_pFileBuf,BUFFER_SIZE);
			
			if((ret=read_buffer(m_pFileBuf,m_nFileBufSize))<BUFFER_SIZE)
			{
				memcpy(m_pFileBuf_tmp+nalu.size,m_pFileBuf,ret);
				nalu.size=nalu.size+ret;
				nalu.data=m_pFileBuf_tmp;
				nalhead_pos=NO_MORE_BUFFER_TO_READ;
				return FALSE;
			}
			naltail_pos=0;
			nalhead_pos=GOT_A_NAL_INCLUDE_A_BUFFER;
			continue;
		}
	}
	return FALSE;  
} 

int RTMP264_Send(int (*read_buffer)(unsigned char *buf, int buf_size))  
{    
	int ret;
	uint32_t now,last_update;
	NaluUnit naluUnit;  
	unsigned int tick = 0;  
	unsigned int tick_gap = 0; 
	int bKeyframe = -1; 
	  
	if(read_buffer == NULL)
	{
		CL_PRINTF("read_buffer function is null!\n");
		return RTMP264_ERROR;
	}
	memset(&metaData,0,sizeof(RTMPMetadata));
	memset(m_pFileBuf,0,BUFFER_SIZE);
	
	if((ret=read_buffer(m_pFileBuf,m_nFileBufSize))<0)
	{
		CL_PRINTF("read buffer size:%d\n",ret);
		return FALSE;
	}

	ReadFirstNaluFromBuf(naluUnit,read_buffer);  
	metaData.nSpsLen = naluUnit.size;  
	metaData.Sps=NULL;
	metaData.Sps=(unsigned char*)malloc(naluUnit.size);
	if(metaData.Sps == NULL)
	{
		CL_PRINTF("malloc metaData.Sps fail!\n");
		return RTMP264_ERROR;
	}
	memset(metaData.Sps,0x00,naluUnit.size);
	memcpy(metaData.Sps,naluUnit.data,naluUnit.size);

	ReadOneNaluFromBuf(naluUnit,read_buffer);  
	metaData.nPpsLen = naluUnit.size; 
	metaData.Pps=NULL;
	metaData.Pps=(unsigned char*)malloc(naluUnit.size);
	if(metaData.Pps == NULL)
	{
		CL_PRINTF("malloc metaData.Pps fail!\n");
		return RTMP264_ERROR;
	}
	memset(metaData.Pps,0x00,naluUnit.size);
	memcpy(metaData.Pps,naluUnit.data,naluUnit.size);
	
	int width = 0,height = 0, fps=0;  
	h264_decode_sps(metaData.Sps,metaData.nSpsLen,width,height,fps);  
	//metaData.nWidth = width;  
	//metaData.nHeight = height;  
	if(fps)
		metaData.nFrameRate = fps; 
	else
		metaData.nFrameRate = 25;

	//ret=SendVideoSpsPps(metaData.Pps,metaData.nPpsLen,metaData.Sps,metaData.nSpsLen);
	//if(ret!=1)
	//	return FALSE;

	tick_gap = 1000/metaData.nFrameRate; 
	ReadOneNaluFromBuf(naluUnit,read_buffer);
	bKeyframe  = (naluUnit.type == 0x05) ? TRUE : FALSE;
	while(SendH264Packet(naluUnit.data,naluUnit.size,bKeyframe,tick))  
	{    
got_sps_pps:
		//if(naluUnit.size==8581)
		CL_PRINTF("NALU size:%8d\n",naluUnit.size);
		last_update=RTMP_GetTime();
		if(!ReadOneNaluFromBuf(naluUnit,read_buffer))
				goto end;
		if(naluUnit.type == 0x07 || naluUnit.type == 0x08)
			goto got_sps_pps;
		bKeyframe  = (naluUnit.type == 0x05) ? TRUE : FALSE;
		tick +=tick_gap;
		now=RTMP_GetTime();
		msleep((tick_gap-now+last_update));  
		//msleep(40);
	}  
end:
	free(metaData.Sps);
	free(metaData.Pps);
	return TRUE;  
}  


