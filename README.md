# Sunflower250701
文本编辑器\n
代码结构：代码包含了display.c,editor.h,file_io.c,input.c,teminal.c,text_operations.c和main.c,Makefile共8个文件\n
makefile的作用是编译程序，我在这里使用了SRCS = $(wildcard *.c)和OBJS = $(SRCS:.c=.o)，可以将所有的.c后缀文件一起编译，方便快捷了很多！\n
display.c用于绘制消息和状态栏、更新屏幕、处理滚动偏移。主要作用是画面处理。\n
file_io.c用于打开txt文件并读取到编辑器中。主要作用是对目标文件进行提取。\n
input.c用于处理用户的输入内容，判断输入的字符或者控制键。\n
teminal让用户可以被即时响应每个按键，切换到"原始模式"，编写了disableRawMode和enableRawMode函数。\n
text_operations实现了编辑的功能，对文本进行修改、删除等操作。\n
editor.h作为一个头文件,可以被多个源文件共享，通过共享，这些源文件可以使用相同的函数声明、常量定义、结构体声明等，保证了代码一致、可复用.\n
main就是主函数,我在这里实现了匹配和control键的功能(保存,退出或者搜索),并运行main函数.\n
