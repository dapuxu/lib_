.SUFFIXES : .x .o .c .s

DIR_INC	= ../inc
DIR_LIB = ../lib

INCLUDE = -I./ \
		  -I$(DIR_INC) \

LDFLAGS = -lpthread -lc -lgcc
LDFLAGS += -L$(DIR_LIB) \
			-lcommonfunc

CFLAGS := $(INCLUDE) -Wall -c -O2

CC=/opt/arm-toolchain/gcc-linaro-7.3.1-2018.05-i686_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-gcc
STRIP=/opt/arm-toolchain/gcc-linaro-7.3.1-2018.05-i686_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-strip

VPATH = %.c
SRC=$(wildcard *.c ./*.c)

#Êä³ö¶Î¶¨Òå
TARGET = demo-server
DIR=$(notdir $(SRC))
OBJ=$(patsubst %.c,%.o,$(DIR))

all:$(TARGET)

$(TARGET):$(OBJ)
	$(CC) $^ -o $@ $(LDFLAGS) -static

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@


.PHONY: clean cleanobj
cleanobj:
	rm  -f $(OBJ)
clean:
	rm  -f $(OBJ) $(TARGET)

