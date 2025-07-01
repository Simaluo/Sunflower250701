#define _DEFAULT_SOURCE//启用默认标准特性
#define _BSD_SOURCE
#define _GNU_SOURCE

#include "editor.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

// 这是全局编辑器状态的唯一定义，在editor.h中用了extern
struct editorConfig E;

//查找

void editorFindCallback(char *query, int key) {
    static int last_match = -1;
    static int direction = 1;

    // 恢复上一个匹配项的高亮
    if (last_match != -1) {
        erow *row = &E.row[last_match];
        memset(row->hl, HL_NORMAL, row->rsize);
    }

    if (key == '\r' || key == '\x1b') {
        last_match = -1;
        direction = 1;
        return;
    } else if (key == ARROW_RIGHT || key == ARROW_DOWN) {//右边或者下，dir变为1
        direction = 1;
    } else if (key == ARROW_LEFT || key == ARROW_UP) {//左或者上，dir变为-1
        direction = -1;
    } else {
        last_match = -1;
        direction = 1;
    }

    if (last_match == -1) direction = 1;
    int current = last_match;
    int i;
    for (i = 0; i < E.numrows; i++) {
        current += direction;
        if (current == -1) current = E.numrows - 1;
        else if (current == E.numrows) current = 0;

        erow *row = &E.row[current];
        char *match = strstr(row->render, query);
        if (match) {
            last_match = current;
            E.cy = current;
            E.cx = match - row->render;
            E.rowoff = E.numrows;

            memset(&row->hl[match - row->render], HL_MATCH, strlen(query));
            break;
        }
    }
}

void editorFind() {
    int saved_cx = E.cx;
    int saved_cy = E.cy;
    int saved_coloff = E.coloff;
    int saved_rowoff = E.rowoff;

    char *query = editorPrompt("Search: %s (Use ESC/Arrows/Enter)", editorFindCallback);

    if (query) {
        free(query);
    } else {
        E.cx = saved_cx;
        E.cy = saved_cy;
        E.coloff = saved_coloff;
        E.rowoff = saved_rowoff;
    }
}

void initEditor() {//初始化编辑器
    E.cx = 0;//光标置零
    E.cy = 0;
    E.rx = 0;
    E.rowoff = 0;
    E.coloff = 0;
    E.numrows = 0;
    E.row = NULL;
    E.dirty = 0;
    E.filename = NULL;
    E.statusmsg[0] = '\0';
    E.statusmsg_time = 0;

    if (getWindowSize(&E.screenrows, &E.screencols) == -1) die("getWindowSize");
    E.screenrows -= 2;
}

void editorProcessKeypress() {//编辑器的控制字符信号
    static int quit_times = KILO_QUIT_TIMES;
    int c = editorReadKey();

    switch (c) {
        case '\r': editorInsertNewline(); break;
        case CTRL_KEY('q')://退出
            if (E.dirty && quit_times > 0) {
                editorSetStatusMessage("WARNING!!! File has unsaved changes. "
                    "Press Ctrl-Q %d more times to quit.", quit_times);//如果便捷了但是没有保存则提示
                quit_times--;
                return;
            }
            write(STDOUT_FILENO, "\x1b[2J", 4);
            write(STDOUT_FILENO, "\x1b[H", 3);
            exit(0);
            break;
        case CTRL_KEY('s'): editorSave(); break;//保存
        case HOME_KEY: E.cx = 0; break;
        case END_KEY:
            if (E.cy < E.numrows) E.cx = E.row[E.cy].size;
            break;
        case CTRL_KEY('f'): editorFind(); break;
        case BACKSPACE:
        case CTRL_KEY('h'):
        case DEL_KEY:
            if (c == DEL_KEY) editorMoveCursor(ARROW_RIGHT);
            editorDelChar();
            break;
        case PAGE_UP:
        case PAGE_DOWN:
            {
                if (c == PAGE_UP) E.cy = E.rowoff;
                else if (c == PAGE_DOWN) {
                    E.cy = E.rowoff + E.screenrows - 1;
                    if (E.cy > E.numrows) E.cy = E.numrows;
                }
                int times = E.screenrows;
                while (times--)
                    editorMoveCursor(c == PAGE_UP ? ARROW_UP : ARROW_DOWN);
            }
            break;
        case ARROW_UP://方向键判断
        case ARROW_DOWN:
        case ARROW_LEFT:
        case ARROW_RIGHT:
            editorMoveCursor(c);
            break;
        case CTRL_KEY('l'):
        case '\x1b':
            break;
        default:
            editorInsertChar(c);
            break;
    }
    quit_times = KILO_QUIT_TIMES;
}

int main(int argc, char *argv[]) {//顶层main函数，实现主要功能
    enableRawMode();
    initEditor();
    if (argc >= 2) {
        editorOpen(argv[1]);
    }

    editorSetStatusMessage("HELP: Ctrl-S = save | Ctrl-Q = quit | Ctrl-F = find");

    while (1) {//通过while循环读入和刷新，读取字符和控制操作和
        editorRefreshScreen();
        editorProcessKeypress();
    }

    return 0;
}
