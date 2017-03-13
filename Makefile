


export PLATFORM=hisi_3518e
export CROSS_COMPILE=/opt/hisi-linux-3518e/x86-arm/arm-hisiv300-linux/bin/arm-hisiv300-linux-uclibcgnueabi-


SUBDIRS += zlib
SUBDIRS += librtmp
SUBDIRS += mbedtls
SUBDIRS += rtmpsend_demo


all:apps

apps:prefix
	for d in $(SUBDIRS);\
	do\
		test ! -d $$d && continue;\
		make -C $$d || exit 1;\
	done
clean:
	for d in $(SUBDIRS);\
	do\
		test ! -d $$d && continue;\
		make -C $$d clean || exit 1;\
	done

install:
	for d in $(SUBDIRS);\
	do\
		test ! -d $$d && continue;\
		make -C $$d install|| exit 1;\
	done

prefix:

