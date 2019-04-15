.PHONY: build papermark
CC = /home/hi/bin/gcc-linaro-arm-linux-gnueabihf-4.8-2013.10_linux/bin/arm-linux-gnueabihf-gcc
AR = /home/hi/bin/gcc-linaro-arm-linux-gnueabihf-4.8-2013.10_linux/bin/arm-linux-gnueabihf-ar
LIBPATH = ../lib
TARGET = libcommonfunc.a

DIR_INC = inc/
DIR_DEBUG = debug/

INCLUDE = \
		  -I$(DIR_INC) \
		  -I$(DIR_DEBUG) \

CFLAGS := ${INCLUDE}

#全局搜索C文件路径
VPATH = %.c $(DIR_DEBUG)

SRCS =${DIR_DEBUG}*.c

DIR=$(notdir $(SRCS))
OBJS=$(patsubst %.c,%.o,$(DIR))

all:$(OBJS)
$(OBJS):$(SRCS)
	$(CC) $(CFLAGS) -c $^
	${AR} rcs $(TARGET) $^
	cp $(TARGET) $(LIBPATH)
	rm -f *.o
clean:
	rm -f *.o
	rm -f ${TARGET}