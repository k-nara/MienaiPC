/* MienaiPC Project: Kota Nara */

#define MIENAI_FLAG 1

#define COL_BLACK       0x00
#define COL_DARKRED     0x80
#define COL_DARKGREEN   0x10
#define COL_DARKBLUE    0x02
#define COL_DARKCYAN    0x12
#define COL_DARKMAGENTA 0x82
#define COL_DARKYELLOW  0x90
#define COL_GRAY        0xb6
#define COL_DARKGRAY    0x6d
#define COL_RED         0xe0
#define COL_GREEN       0x1c
#define COL_BLUE        0x03
#define COL_CYAN        0x1f
#define COL_MAGENTA     0xe3
#define COL_YELLOW      0xfc
#define COL_WHITE       0xff

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
