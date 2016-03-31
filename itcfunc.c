/* MienaiPC Project: Kota Nara */

#include <stdlib.h>
#include <time.h>
#include <curses.h>
#include <stdarg.h>
#include "itcfunc.h"
#define unused(var) (void)(var)

WINDOW* the_window;

int lcd_ttyopen(int rotate)
{
    unused(rotate);

    the_window = initscr();
    start_color();

    nodelay(the_window, TRUE);
    cbreak();
    noecho();
    curs_set(0);

    init_pair(COL_RED,     COLOR_RED,     COLOR_BLACK);
    init_pair(COL_GREEN,   COLOR_GREEN,   COLOR_BLACK);
    init_pair(COL_YELLOW,  COLOR_YELLOW,  COLOR_BLACK);
    init_pair(COL_BLUE,    COLOR_BLUE,    COLOR_BLACK);
    init_pair(COL_MAGENTA, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(COL_CYAN,    COLOR_CYAN,    COLOR_BLACK);
    init_pair(COL_WHITE,   COLOR_WHITE,   COLOR_BLACK);
    init_pair(COL_BLACK,   COLOR_BLACK,   COLOR_BLACK);
    bkgd(COLOR_PAIR(COL_WHITE));

    erase();
    refresh();

    return rotate;
}

void lcd_ttyclose() { endwin(); }

void lcd_setcolor(int color) { attrset(COLOR_PAIR(color)); }

void lcd_locate(int x, int y) { move(y, x); }

int lcd_printf(char* fmt, ...)
{
    va_list lst;
    int retval;
    va_start(lst, fmt);
    retval = vwprintw(the_window, fmt, lst);
    refresh();
    return retval;
}

void lcd_nextline() { printw("\n"); }

void lcd_cls() { erase(); refresh(); }

int counter_get() { return clock() * 100 * 1000 / CLOCKS_PER_SEC; }

char key_get()
{
    switch(getch())
    {
      case 'w': return 0x20;
      case 's': return 0x10;
      case 'a': return 0x08;
      case 'd': return 0x04;
      case 'j': return 0x02;
      case 'k': return 0x01;
      case 'q': lcd_ttyclose(); exit(0);
      default:  return 0;
    }
}

/* Unsupported (for now). */
void lcd_setshowpage(int page) { unused(page); }
void lcd_setdrawpage(int page) { unused(page); }
void lcd_copypage(int src, int dst) { unused(src), unused(dst); }
void lcd_settransparent(int flag) { unused(flag); }
