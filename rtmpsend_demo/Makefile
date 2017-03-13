TOP_DIR=$(shell pwd)
SRC_DIR=$(TOP_DIR)/demo
BASE_LIB=
LIBRTMP_DIR=../../librtmp
ZLIB_DIR=../../zlib/zlib-1.2.11
MBEDTLS_DIR=../../mbedtls

all:
	make -C $(SRC_DIR) XCFLAGS+=-I$(LIBRTMP_DIR)/rtmpdump/librtmp/ XCFLAGS+=-I$(MBEDTLS_DIR)/mbedtls-2.3.0/include/ XLDFLAGS+=$(LIBRTMP_DIR)/librtmp.a XLDFLAGS+=$(MBEDTLS_DIR)/libmbedtls.a XLDFLAGS+=$(MBEDTLS_DIR)/libmbedx509.a XLDFLAGS+=$(MBEDTLS_DIR)/libmbedcrypto.a 

install:
	#make -C $(SRC_DIR) install

clean:
	make -C $(SRC_DIR) clean

distclean:
