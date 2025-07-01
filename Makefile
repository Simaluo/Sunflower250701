#现学现用的一键编译！

CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c99 -g

TARGET = main

# 自动查找所有c文件作为源文件
SRCS = $(wildcard *.c)
# 根据源文件生成对应的o文件列表
OBJS = $(SRCS:.c=.o)

all: $(TARGET)
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

# 清理规则：删除所有生成的目标文件和可执行文件
clean:
	rm -f $(TARGET) $(OBJS)


.PHONY: all clean
