#CROSS_COMPILE=/opt/hisi-linux-3518e/x86-arm/arm-hisiv300-linux/bin/arm-hisiv300-linux-uclibcgnueabi-
CC = $(CROSS_COMPILR)
CC = $(CROSS_COMPILE)gcc
CPP = $(CROSS_COMPILE)g++
LD = $(CROSS_COMPILE)ld
AR = $(CROSS_COMPILE)ar
STRIP = $(CROSS_COMPILE)strip

CFLAGS +=$(XCFLAGS)
CFLAGS += -I.
CFLAGS += -D__CL_DEBUG_ENABLE__

LDFLAGS +=$(XLDFLAGS)
LDFLAGS += -ldl

BIN = rtmpsend

OBJS += $(patsubst %.c,%.o,$(wildcard *.c))
OBJS += $(patsubst %.cpp,%.o,$(wildcard *.cpp))
OBJS += $(patsubst %.cc,%.o,$(wildcard *.cc))


all:$(BIN)

%.o:%.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<
%.o:%.cpp
	$(CPP) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

$(BIN):$(OBJS)
	$(CPP) -o $@ $^ $(LDFLAGS)

install:
	$(STRIP) $(BIN)

clean:
	rm -rf *.o $(BIN)
