TOP_DIR=$(shell pwd)
SRC_DIR=$(TOP_DIR)/rtmpdump
BASE_LIB= 
ZLIBDIR	=../../../zlib/zlib-1.2.11/
ZLIB_LIB=../../zlib/zlib-1.2.11/


all:
	make -C $(SRC_DIR) SYS=posix SHARED= CRYPTO= INC+=-I$(ZLIBDIR)/ XLDFLAGS+=-L$(ZLIB_LIB)
	cp -af $(SRC_DIR)/librtmp/librtmp.a .

install:
	cp -af $(SRC_DIR)/librtmp/librtmp.a .

clean:
	make -C $(SRC_DIR) clean
