#定义编译器及其选项
CC = arm-linux-gnueabihf-gcc
//CC = gcc
CFLAGS = -Wall -g
#LDFLAGS = -lsqlite3 -lcgic

LDFLAGS = -L/usr/local/lib -lsqlite3 -lcgic

# 目标可执行程序名称
TARGET1 = csystemInfo
TARGET2 = csurveyInfo
TARGET3 = cuserInfo
TARGET4 = cmeshInfo
TARGET5 = cserialInfo
TARGET6 = ctopologyInfo
TARGET7 = crouteInfo
TARGET8 = ctimeslot
TARGET9 = cbootInfo
TARGET10 = cimageInfo
TARGET11 = crebootInfo
TARGET12 = cfreqInfo

#拷贝生成的可执行程序到指定的cgi目录
//OUTPUT_DIR = /home/gby/cp/www/cgi-bin/ 
OUTPUT_DIR = /home/zynq/boa-banka/www/cgi-bin/ 

# 源文件列表
SRCS1 := csystemInfo.c
SRCS2 := csurveyInfo.c
SRCS3 := cuserInfo.c
SRCS4 := cmeshInfo.c
SRCS5 := cserialInfo.c
SRCS6 := ctopologyInfo.c
SRCS7 := crouteInfo.c
SRCS8 := ctimeslot.c
SRCS9 := cbootInfo.c
SRCS10 := cimageInfo.c
SRCS11 := crebootInfo.c
SRCS12 := cfreqInfo.c
OBJS1 := $(patsubst %.c,%.o,$(SRCS1))
OBJS2 := $(patsubst %.c,%.o,$(SRCS2))
OBJS3 := $(patsubst %.c,%.o,$(SRCS3))
OBJS4 := $(patsubst %.c,%.o,$(SRCS4))
OBJS5 := $(patsubst %.c,%.o,$(SRCS5))
OBJS6 := $(patsubst %.c,%.o,$(SRCS6))
OBJS7 := $(patsubst %.c,%.o,$(SRCS7))
OBJS8 := $(patsubst %.c,%.o,$(SRCS8))
OBJS9 := $(patsubst %.c,%.o,$(SRCS9))
OBJS10 := $(patsubst %.c,%.o,$(SRCS10))
OBJS11 := $(patsubst %.c,%.o,$(SRCS11))
OBJS12 := $(patsubst %.c,%.o,$(SRCS12))

all: $(TARGET1) $(TARGET2) $(TARGET3) $(TARGET4) $(TARGET5) $(TARGET6) $(TARGET7) $(TARGET8) $(TARGET9) $(TARGET10) $(TARGET11) $(TARGET12)

$(TARGET1): $(OBJS1)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)
	cp $(TARGET1)  $(OUTPUT_DIR) 

$(TARGET2): $(OBJS2)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)
	cp $(TARGET2)  $(OUTPUT_DIR) 
$(TARGET3): $(OBJS3)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)
	cp $(TARGET3)  $(OUTPUT_DIR) 
$(TARGET4): $(OBJS4)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)
	cp $(TARGET4)  $(OUTPUT_DIR) 
$(TARGET5): $(OBJS5)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)
	cp $(TARGET5)  $(OUTPUT_DIR) 
$(TARGET6): $(OBJS6)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)
	cp $(TARGET6)  $(OUTPUT_DIR) 
$(TARGET7): $(OBJS7)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)
	cp $(TARGET7)  $(OUTPUT_DIR) 
$(TARGET8): $(OBJS8)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)
	cp $(TARGET8)  $(OUTPUT_DIR) 
$(TARGET9): $(OBJS9)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)
	cp $(TARGET9)  $(OUTPUT_DIR) 
$(TARGET10): $(OBJS10)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)
	cp $(TARGET10)  $(OUTPUT_DIR) 
$(TARGET11): $(OBJS11)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)
	cp $(TARGET11)  $(OUTPUT_DIR) 
$(TARGET12): $(OBJS12)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)
	cp $(TARGET12)  $(OUTPUT_DIR) 
clean:
	rm -f $(TARGET1) *.o
	rm -f $(TARGET2) *.o
	rm -f $(TARGET3) *.o
	rm -f $(TARGET4) *.o
	rm -f $(TARGET5) *.o
	rm -f $(TARGET6) *.o
	rm -f $(TARGET7) *.o
	rm -f $(TARGET8) *.o
	rm -f $(TARGET9) *.o
	rm -f $(TARGET10) *.o
	rm -f $(TARGET11) *.o
	rm -f $(TARGET12) *.o
