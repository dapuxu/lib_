.PHONY: build papermark
CC = /opt/arm-toolchain/gcc-linaro-7.3.1-2018.05-i686_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-gcc
AR = /opt/arm-toolchain/gcc-linaro-7.3.1-2018.05-i686_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-ar

#CC = gcc
#AR = ar

LIBPATH = ../lib
INCPATH = ../inc
TARGET = libcommonfunc.a

DIR_INC = inc/
DIR_DEBUG = debug/
DIR_LIST = list/
DIR_NET = network/
DIR_STR = string/
DIR_CJSON = cjson/

INCLUDE = \
		  -I$(DIR_INC) \
		  -I$(DIR_DEBUG) \
		  -I$(DIR_LIST) \
		  -I$(DIR_NET) \
		  -I$(DIR_STR) \
		  -I$(DIR_CJSON)

CFLAGS := ${INCLUDE}

SRCS =${DIR_DEBUG}*.c
SRCS += ${DIR_LIST}*.c
SRCS += ${DIR_NET}*.c
SRCS += ${DIR_STR}*.c
SRCS += ${DIR_CJSON}*.c

OBJS = lib_debug.o lib_list.o lib_net.o lib_string.o lib_cjson.o

$(OBJS):$(SRCS)
	$(CC) $(CFLAGS) -c $^
#	$(CC) $(CFLAGS) -c $< -o $@
all:$(OBJS)
	${AR} rcs $(TARGET) $^
	cp $(TARGET) $(LIBPATH)
	cp ${DIR_INC}*.h ${INCPATH}
	cp ${DIR_DEBUG}*.h ${INCPATH}
	cp ${DIR_LIST}*.h ${INCPATH}
	cp ${DIR_NET}*.h ${INCPATH}
	cp ${DIR_STR}*.h ${INCPATH}
	cp ${DIR_CJSON}*.h ${INCPATH}
	#rm -f *.o
clean:
	rm -f *.o
	rm -f ${TARGET}
