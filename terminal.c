#include "editor.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>


void die(const char *s) {
    // 使用 ANSI 转义序列清屏并将光标归位
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);

    // perror 会根据全局变量 errno 的值，打印出对应的系统错误信息
    perror(s);
    // 退出程序，返回状态 1 表示失败
    exit(1);
}


void disableRawMode() {
    // tcsetattr 用于设置终端属性
    // TCSAFLUSH 参数表示等待所有待处理的输出都被写入后，再应用新的终端设置，并且丢弃所有未读取的输入
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios) == -1) {
        die("tcsetattr");
    }
}

void enableRawMode() {
    if (tcgetattr(STDIN_FILENO, &E.orig_termios) == -1) die("tcgetattr");
    atexit(disableRawMode);

    // 创建一个新的 termios 结构体，基于原始设置进行修改
    struct termios raw = E.orig_termios;

    // IXON: 启用软件流控制（Ctrl-S 和 Ctrl-Q）关闭它
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);

    // c_oflag: 输出标志
    // OPOST: 开启输出处理（例如，将 '\n' 转换为 '\r\n'）关闭
    raw.c_oflag &= ~(OPOST);

    // c_cflag: 控制标志
    raw.c_cflag |= (CS8);

    // c_lflag: 本地标志
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

    // c_cc: 控制字符数组
    // VTIME: 在非规范模式下，read() 调用在返回前的超时时间设置为1，即100毫秒
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;

    // 应用修改后的终端属性
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) die("tcsetattr");
}

int getWindowSize(int *rows, int *cols) {
    struct winsize ws;

    // ioctl 是一个用于设备 I/O 操作的系统调用
    // TIOCGWINSZ 用于获取窗口大小
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
        return -1;
    } else {
        *cols = ws.ws_col;
        *rows = ws.ws_row;
        return 0;
    }
}