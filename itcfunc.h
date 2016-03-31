/* MienaiPC Project: Kota Nara */

#define MIENAI_FLAG 1

#define COL_RED         1
#define COL_DARKRED     1
#define COL_GREEN       2
#define COL_DARKGREEN   2
#define COL_YELLOW      3
#define COL_DARKYELLOW  3
#define COL_BLUE        4
#define COL_DARKBLUE    4
#define COL_MAGENTA     5
#define COL_DARKMAGENTA 5
#define COL_CYAN        6
#define COL_DARKCYAN    6
#define COL_WHITE       7
#define COL_GRAY        7
#define COL_BLACK       8
#define COL_DARKGRAY    8

int lcd_ttyopen(int);
void lcd_ttyclose();
void lcd_setcolor(int);
void lcd_setshowpage(int);
void lcd_setdrawpage(int);
void lcd_copypage(int, int);
void lcd_cls();
void lcd_setreverse(int);
void lcd_settransparent(int);
void lcd_locate(int, int);
int lcd_printf(char* fmt, ...);
void lcd_nextline();

int counter_get();
char key_get();
