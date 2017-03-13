TOP_DIR=$(shell pwd)
SRC_DIR=$(TOP_DIR)/zlib-1.2.11
BASE_LIB= 

all:
	#$(SRC_DIR)/configure --prefix=$(TOP_DIR)/install/;
	#test -f $(SRC_DIR)/Makefile || { cd $(SRC_DIR);CC=$(CROSS_COMPILE)gcc;$(SRC_DIR)/configure --prefix=$(SRC_DIR)/install/;}

	make -C $(SRC_DIR)

install:
	cp -af $(SRC_DIR)/libz.a $(TOP_DIR)/

clean:
	make -C $(SRC_DIR) clean

distclean:
	rm $(SRC_DIR)/Makefile
#	make -C $(SRC_DIR) distclean
