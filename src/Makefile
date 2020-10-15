.PHONY: build papermark
CC = /opt/arm-toolchain/gcc-linaro-7.3.1-2018.05-i686_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-gcc
AR = /opt/arm-toolchain/gcc-linaro-7.3.1-2018.05-i686_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-ar
LIBPATH = ../lib
INCPATH = ../inc
TARGET = libcommonfunc.a

DIR_INC = inc/
DIR_DEBUG = debug/
DIR_LIST = list/
DIR_NET = network/
DIR_STR = string/

INCLUDE = \
		  -I$(DIR_INC) \
		  -I$(DIR_DEBUG) \
		  -I$(DIR_LIST) \
		  -I$(DIR_NET) \
		  -I$(DIR_STR) \

LDFLAGS = -lpthread -lc -lgcc

CFLAGS := ${INCLUDE}

SRCS =${DIR_DEBUG}*.c
SRCS += ${DIR_LIST}*.c
SRCS += ${DIR_NET}*.c
SRCS += ${DIR_STR}*.c

OBJS = lib_debug.o lib_list.o lib_net.o lib_string.o

$(OBJS):$(SRCS)
	$(CC) $(CFLAGS) -c $^ $(LDFLAGS) -static

all:$(OBJS)
	${AR} rcs $(TARGET) $^
	cp $(TARGET) $(LIBPATH)
	cp ${DIR_INC}*.h ${INCPATH}
	cp ${DIR_DEBUG}*.h ${INCPATH}
	cp ${DIR_LIST}*.h ${INCPATH}
	cp ${DIR_NET}*.h ${INCPATH}
	cp ${DIR_STR}*.h ${INCPATH}
	#rm -f *.o
clean:
	rm -f *.o
	rm -f ${TARGET}