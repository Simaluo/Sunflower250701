#include "editor.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdlib.h>


void abAppend(struct abuf *ab, const char *s, int len) {
    char *new = realloc(ab->b, ab->len + len); // 为新内容重新分配内存，大小为旧长度 + 新长度
    if (new == NULL) return;// 防御
    // 内存分配失败则返回
    // 然后将新字符串 s 拷贝到缓冲区的末尾
    memcpy(&new[ab->len], s, len);
    ab->b = new;
    ab->len += len;
}

void abFree(struct abuf *ab) {
    free(ab->b);//free缓冲行函数
}


void editorScroll() {//光标的位置检查，调整滚动
    E.rx = 0;
    if (E.cy < E.numrows) {
        E.rx = editorRowCxToRx(&E.row[E.cy], E.cx);
    }

    if (E.cy < E.rowoff) {
        E.rowoff = E.cy;
    }
    if (E.cy >= E.rowoff + E.screenrows) {
        E.rowoff = E.cy - E.screenrows + 1;
    }
    if (E.rx < E.coloff) {
        E.coloff = E.rx;
    }
    if (E.rx >= E.coloff + E.screencols) {
        E.coloff = E.rx - E.screencols + 1;
    }
}

int editorSyntaxToColor(int hl) {
    switch (hl) {
        case HL_MATCH: return 34; //蓝色
        default: return 37; //白色
    }
}

void editorDrawRows(struct abuf *ab) {
    int y;
    for (y = 0; y < E.screenrows; y++) {
        int filerow = y + E.rowoff; // 计算当前要绘制的文件行号
        if (filerow >= E.numrows) {
            // 如果文件为空，在屏幕中央绘制欢迎信息
            if (E.numrows == 0 && y == E.screenrows / 3) {
                char welcome[80];
                int welcomelen = snprintf(welcome, sizeof(welcome),
                    "Kilo editor -- version %s", KILO_VERSION);
                if (welcomelen > E.screencols) welcomelen = E.screencols;
                int padding = (E.screencols - welcomelen) / 2;
                if (padding) {
                    abAppend(ab, "~", 1);
                    padding--;
                }
                while (padding--) abAppend(ab, " ", 1);
                abAppend(ab, welcome, welcomelen);
            } else {
                // 在文件内容结束后的行首绘制波浪号
                abAppend(ab, "~", 1);
            }
        } else {
            // 绘制文件中的行
            int len = E.row[filerow].rsize - E.coloff;
            if (len < 0) len = 0;
            if (len > E.screencols) len = E.screencols;
            char *c = &E.row[filerow].render[E.coloff];
            unsigned char *hl = &E.row[filerow].hl[E.coloff];
            int current_color = -1;
            int j;
            for (j = 0; j < len; j++) {
                // 根据 hl 数组中的高亮信息设置颜色
                if (hl[j] == HL_NORMAL) {
                    if (current_color != -1) {
                        abAppend(ab, "\x1b[39m", 5); // 恢复默认颜色
                        current_color = -1;
                    }
                    abAppend(ab, &c[j], 1);
                } else {
                    int color = editorSyntaxToColor(hl[j]);
                    if (color != current_color) {
                        current_color = color;
                        char buf[16];
                        int clen = snprintf(buf, sizeof(buf), "\x1b[%dm", color);
                        abAppend(ab, buf, clen); // 设置新颜色
                    }
                    abAppend(ab, &c[j], 1);
                }
            }
            abAppend(ab, "\x1b[39m", 5); // 在行尾恢复默认颜色
        }
        // 清除从光标到行尾的内容，以防旧内容残留
        abAppend(ab, "\x1b[K", 3);
        abAppend(ab, "\r\n", 2); // 换行
    }
}


void editorDrawStatusBar(struct abuf *ab) {
    abAppend(ab, "\x1b[7m", 4);//开启反色
    char status[80], rstatus[80]; // 左侧状态：文件名、行数、修改状态
    int len = snprintf(status, sizeof(status), "%.20s - %d lines %s",
        E.filename ? E.filename : "[No Name]", E.numrows,
        E.dirty ? "(modified)" : "");
    int rlen = snprintf(rstatus, sizeof(rstatus), "%d/%d",    // 右侧状态：当前行号/总行数
        E.cy + 1, E.numrows);
    if (len > E.screencols) len = E.screencols;
    abAppend(ab, status, len);    
    // 填充空格，使右侧状态靠右对齐
    while (len < E.screencols) {
        if (E.screencols - len == rlen) {
            abAppend(ab, rstatus, rlen);
            break;
        } else {
            abAppend(ab, " ", 1);
            len++;
        }
    }
    abAppend(ab, "\x1b[m", 3);
    abAppend(ab, "\r\n", 2);
}

void editorDrawMessageBar(struct abuf *ab) {
    abAppend(ab, "\x1b[K", 3);
    int msglen = strlen(E.statusmsg);
    if (msglen > E.screencols) msglen = E.screencols;
    if (msglen && time(NULL) - E.statusmsg_time < 5)
        abAppend(ab, E.statusmsg, msglen);
}

void editorRefreshScreen() {
    editorScroll(); // 首先计算滚动位置
    struct abuf ab = ABUF_INIT; // 初始化一个缓冲行
    abAppend(&ab, "\x1b[?25l", 6); // 先隐藏光标
    abAppend(&ab, "\x1b[H", 3);    // 将光标移动到屏幕左上角
    editorDrawRows(&ab);         // 绘制所有文本行
    editorDrawStatusBar(&ab);    // 绘制状态栏
    editorDrawMessageBar(&ab);   // 绘制消息栏

    char buf[32];
    snprintf(buf, sizeof(buf), "\x1b[%d;%dH", (E.cy - E.rowoff) + 1,(E.rx - E.coloff) + 1);
    abAppend(&ab, buf, strlen(buf));
    abAppend(&ab, "\x1b[?25h", 6);
    write(STDOUT_FILENO, ab.b, ab.len);
    abFree(&ab);
}

void editorSetStatusMessage(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(E.statusmsg, sizeof(E.statusmsg), fmt, ap);// 将格式化后的字符串存入 E.statusmsg
    va_end(ap);
    E.statusmsg_time = time(NULL);
}
