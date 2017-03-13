
#CYLAN rtmp demo code
http://www.cylan.com.cn/

=============================
Compile:
	1. 修改编译平台
		PLATFORM=hisi_3518e
		CROSS_COMPILE=/opt/hisi-linux-3518e/x86-arm/arm-hisiv300-linux/bin/arm-hisiv300-linux-uclibcgnueabi-
		
	2. 添加 mbedtls 编译平台下的toolchain
		cp hisi_3518e.toolchain.cmake ${PLATFORM}.toolchain.cmake
		
		添加对应平台的platform 工具目录
		if [[ "${PLATFORM}" == "hisi_3518e" ]];then
			export PATH=/opt/hisi-linux-3518e/x86-arm/arm-hisiv300-linux/target/bin/:${INITPATH}
		fi
		
	3. 在顶层目录编译demo 
		$ make

		
Modify:

	1. 升级
		demo 采用的开源库是 rtmpdump-2.4 & mbedtls-2.3.0 & zlib-1.2.11
		今后如若想升级开源库，只需要去下载相应的开源库，解压到相应的 librtmp   mbedtls zlib 目录下
		并修改Makefile 目录中相应编译的版本，从新编译即可
		
	2. 修改democode
		次demo 针对 Cylan hisi3518 摄像头开发测试，测试针对固定h264视屏文件，推流到指定服务器
		
		针对开发厂商可能需要更改一下相应参数，从服务器获取推流 url 地址
		#define ROBOT_SERVER_PORT "443"
		#define ROBOT_SERVER_NAME "www.robotscloud.com (此处填入服务器地址)"
		#define ROBOT_SERVICE_KEY "此处填入用户在萝卜头注册、开通云存储服务的 key"
		#define VID         "此处填入用户在萝卜头注册的企业ID"
		#define SN          "此处填入鉴别设备的唯一序列号"
		#define ACCOUNT     "此处填入设备使用者的账户"
		
		由于测试使用https 从服务器获取推流地址，因此开发者需要修改相应的测试参数，
		从自己的平台获取相应的url 地址用于推流
	
	3. 编译运行
		编译完成后，拷贝到自己的开发平台，并拷贝待发送测试的 video.h264 测试文件到同一目录下
		运行
		./rtmpsend

Tips:
	此demo 采用https 获取 url 地址，解析json 返回数据，因为每个服务器返回数据格式可能存在差异，
	开发者需要注意用cJson解析出自己的rtmp 推流地址填入 url 变量用于建立链接
	
	关键函数在
		RTMP264_Send()
	由于此视屏文件是包含头帧视屏参数sps pps 所以自己解析了视屏的参数属性，并填如了sps pps 用于发送
	如果开发者推送的视屏流是完整的h264 帧的数据格式，只需要在每次I帧的时候发送相应的更新数据 sps pps 即可
	既可以直接调用Sendh264Packet() 发送视屏帧数据
	推流数据第一帧必须为I帧.
	
		

致谢:
	本demo 在Cylan hisi3518 平台正确运行.
	
	https://github.com/latelee/H264BSAnalyzer	感谢开发者提供的win 下h264帧格式解析器
	http://blog.csdn.net/leixiaohua1020		感谢作者推流工作做出的贡献




