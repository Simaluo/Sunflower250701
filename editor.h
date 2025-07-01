#ifndef EDITOR_H
#define EDITOR_H

#include <termios.h>
#include <time.h>

#define KILO_VERSION "0.0.1" //空文件时显示的“版本号”？在github上看到了类似的参考
#define KILO_TAB_STOP 8
#define KILO_QUIT_TIMES 3

#define CTRL_KEY(k) ((k) & 0x1f)

enum editorKey {
  BACKSPACE = 127,  //退格的ASCII
  ARROW_LEFT = 1000,
  ARROW_RIGHT,//下面几个会从 ARROW_LEFT 自动递增
  ARROW_UP,
  ARROW_DOWN,
  DEL_KEY,
  HOME_KEY,
  END_KEY,
  PAGE_UP,
  PAGE_DOWN
};

enum editorHighlight {//高亮，作为提示
  HL_NORMAL = 0,//普通
  HL_MATCH//特殊
};


typedef struct erow {//文本的行结构体定义！
    int size;//原大小
    int rsize;//考虑了tab等特殊键的大小
    char *chars;//原字符的指针
    char *render;//考虑了tab等特殊键的指针
    unsigned char *hl;//highlight指针，区分特殊字符
} erow;

struct editorConfig {//编辑器的总状态定义，包括光标位置、编辑器状态等
    int cx, cy;//光标的水平和垂直表示
    int rx;//考虑了tab的水平表示
    int rowoff;//这两个是滚动偏移
    int coloff;
    int screenrows;//屏幕的行列
    int screencols;
    int numrows;
    erow *row;
    int dirty; //文件修改的标志，0就是未修改
    char *filename;
    char statusmsg[80];
    time_t statusmsg_time;
    struct termios orig_termios;
};

extern struct editorConfig E; //可被其他文件访问的总状态情况

struct abuf {//行文本的缓冲区结构体
  char *b;
  int len;
};

#define ABUF_INIT {NULL, 0}
void abAppend(struct abuf *ab, const char *s, int len);
void abFree(struct abuf *ab);
void editorSetStatusMessage(const char *fmt, ...);
void editorRefreshScreen();
char *editorPrompt(char *prompt, void (*callback)(char *, int));
void die(const char *s);
void disableRawMode();
void enableRawMode();
int getWindowSize(int *rows, int *cols);
int editorReadKey();
void editorMoveCursor(int key);
int editorRowCxToRx(erow *row, int cx);
void editorUpdateRow(erow *row);
void editorInsertRow(int at, char *s, size_t len);
void editorFreeRow(erow *row);
void editorDelRow(int at);
void editorRowInsertChar(erow *row, int at, int c);
void editorRowDelChar(erow *row, int at);
void editorRowAppendString(erow *row, char *s, size_t len);
void editorInsertChar(int c);
void editorInsertNewline();
void editorDelChar();
char *editorRowsToString(int *buflen);
void editorOpen(char *filename);
void editorSave();
void editorFind();
void editorScroll();
int editorSyntaxToColor(int hl);
void editorDrawRows(struct abuf *ab);
void editorDrawStatusBar(struct abuf *ab);
void editorDrawMessageBar(struct abuf *ab);
void initEditor();

#endif // EDITOR
