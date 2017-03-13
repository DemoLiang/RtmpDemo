#define RTMP264_FAIL	-1
#define RTMP264_OK		0
#define RTMP264_ERROR	-1

#define RTMP_HEAD_SIZE   (sizeof(RTMPPacket)+RTMP_MAX_HEADER_SIZE)
#define BUFFER_SIZE 32768
#define GOT_A_NAL_CROSS_BUFFER BUFFER_SIZE+1
#define GOT_A_NAL_INCLUDE_A_BUFFER BUFFER_SIZE+2
#define NO_MORE_BUFFER_TO_READ BUFFER_SIZE+3

typedef struct _NaluUnit  
{  
	int type;  
    int size;  
	unsigned char *data;  
}NaluUnit;

typedef struct _RTMPMetadata  
{  
	// video, must be h264 type   
	unsigned int    nWidth;  
	unsigned int    nHeight;  
	unsigned int    nFrameRate;      
	unsigned int    nSpsLen;  
	unsigned char   *Sps;  
	unsigned int    nPpsLen;  
	unsigned char   *Pps;   
} RTMPMetadata,*LPRTMPMetadata;  

enum  
{  
	 VIDEO_CODECID_H264 = 7,  
};  


int RTMP264_Connect(const char* url);    
    

int RTMP264_Send(int (*read_buffer)(unsigned char *buf, int buf_size));

   
void RTMP264_Close();  


int RTMP264_Init();
int RTMP264_GetUrl(char *url,int len);
int read_buffer_callback(unsigned char *buf, int buf_size );
