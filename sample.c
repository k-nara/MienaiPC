#include <stdlib.h>             /* rand, srand */
#include <string.h>             /* strlen */
#include "itcfunc.h"

/* ---------------------------------------------------------------------
 * OPTIONS
 * --------------------------------------------------------------------- */

#define MAZE_SIZE_MAX 19

/* ---------------------------------------------------------------------
 * CONSTANTS
 * --------------------------------------------------------------------- */

#define WIDTH  (39 + 1)         /* Screen width */
#define HEIGHT (14 + 1)         /* Screen height */
#define COUNTS_PER_MSEC 100     /* Counts per a millisecond */

/* ---------------------------------------------------------------------
 * UTIL: KEY INPUT
 * --------------------------------------------------------------------- */

/* Poll keyboard inputs. */
int current_keys, last_keys = 0;
void keys_update()
{
    int t = key_get();
    current_keys = t & (~ last_keys);
    last_keys = t;
}

/* Test if a key is pressed or not. */
#define INPUT_A (current_keys & 0x08)
#define INPUT_S (current_keys & 0x10)
#define INPUT_W (current_keys & 0x20)
#define INPUT_D (current_keys & 0x04)
#define INPUT_J (current_keys & 0x02)
#define INPUT_K (current_keys & 0x01)

/* ---------------------------------------------------------------------
 * UTIL: TIMER
 * --------------------------------------------------------------------- */

typedef int timer;

/* Make a timer which exceeds in MSEC milliseconds. */
#define new_timer(msec) (counter_get() + msec * COUNTS_PER_MSEC)

/* Get remaining time (in milliseconds) of TIMER. */
#define remaining_time(timer) ((timer - counter_get()) / COUNTS_PER_MSEC)

/* ---------------------------------------------------------------------
 * UTIL: DRAW SCREEN
 * --------------------------------------------------------------------- */

char lcd_screen[HEIGHT][WIDTH], draw_screen[HEIGHT][WIDTH];

/* Clear both LCD and draw-screen. */
void init_draw_screen()
{
    int x, y;

    lcd_cls();

    for(y = 0; y < HEIGHT; y++)
        for(x = 0; x < WIDTH; x++)
            lcd_screen[y][x] = draw_screen[y][x] = ' ';
}

/* Print draw-screen to LCD and clear draw-screen. SET COLOR BEFORE
 * CALLING THIS FUNCTION. */
void screen_flip()
{
    int x, y;

    for(y = 0; y < HEIGHT; y++)
        for(x = 0; x < WIDTH; x++)
        {
            if(lcd_screen[y][x] != draw_screen[y][x])
            {
                lcd_locate(x, y);
                lcd_printf("%c", lcd_screen[y][x] = draw_screen[y][x]);
            }
            draw_screen[y][x] = ' ';
        }
}

/* Copy LCD to draw-screen. */
void load_screen()
{
    int x, y;

    for(y=0; y<HEIGHT; y++)
        for(x=0; x<WIDTH; x++)
            draw_screen[y][x] = lcd_screen[y][x];
}

/* Put STR on draw-screen. */
void draw(const char* str, int x, int y)
{
    while(*str)
        draw_screen[y][x++] = *(str++);
}

/* Draw horizontally flipped. */
void draw_mirr(const char* str, int x, int y)
{
    int t = strlen(str);
    while(t--)
        draw_screen[y][x++] = str[t];
}

/* Draw transparently (ignoring whitespaces). */
void draw_trans(const char* str, int x, int y)
{
    while(*str)
    {
        if(*str != ' ') draw_screen[y][x] = *str;
        x++, str++;
    }
}

/* Draw flipped transparently. */
void draw_mirr_trans(const char* str, int x, int y)
{
    int t = strlen(str);

    while(t--)
    {
        if(str[t] != ' ') draw_screen[y][x] = str[t];
        x++;
    }
}

/* ---------------------------------------------------------------------
 * THE GAME STATE
 * --------------------------------------------------------------------- */

int x_pos, y_pos;
char remaining_steps, kowai = 0;

/* ---------------------------------------------------------------------
 * MAZE GENERATOR
 * --------------------------------------------------------------------- */

char maze_size = 11;            /* must be an odd number */
char maze[MAZE_SIZE_MAX][MAZE_SIZE_MAX + 1]; /* maze[*][maze_size] == '\0' */

/* Access maze safely. */
#define MAZE(y, x) ( x >= 0 && x < maze_size && y >= 0 && y < maze_size ? maze[y][x] : '*' )

/* (internal recursive function for maze_gen) */
void maze_gen_r(int y, int x)
{
    char direction = 0, flags = 0xf, t, dx, dy, i, j;

    while(flags)
    {
        t = rand() % 4;

        /* skip "direction" which is already used */
        while(t--)
            do direction = (direction + 1) % 4;
            while(!((flags >> direction) & 1));

        flags &= ~(1 << direction);

        /* recursion */
        switch(direction)
        {
          case 0:
            if(y + 2 < maze_size - 1 && maze[y + 2][x] == '*')
            {
                maze[y + 1][x] = maze[y + 2][x] = ' ';
                maze_gen_r(y + 2, x);
            }
            break;

          case 1:
            if(x + 2 < maze_size - 1 && maze[y][x + 2] == '*')
            {
                maze[y][x + 1] = maze[y][x + 2] = ' ';
                maze_gen_r(y, x + 2);
            }
            break;

          case 2:
            if(y - 2 > 0 && maze[y - 2][x] == '*')
            {
                maze[y - 1][x] = maze[y - 2][x] = ' ';
                maze_gen_r(y - 2, x);
            }
            break;

          case 3:
            if(x - 2 > 0 && maze[y][x - 2] == '*')
            {
                maze[y][x - 1] = maze[y][x - 2] = ' ';
                maze_gen_r(y, x - 2);
            }
            break;
        }
    }
}

/* Generate a random maze from SEED. */
void maze_gen(unsigned int seed)
{
    int x, y;

    srand(seed);

    /* clear the array */
    for(y = 0; y < maze_size; y++)
    {
        for(x = 0; x < maze_size; x++) maze[y][x] = '*';
        maze[y][x] = '\0';
    }

    /* choose the initial position (must be odd) */
    x = rand() % (maze_size - 1), y = rand() % (maze_size - 1);
    if(!(x % 2)) x++;
    if(!(y % 2)) y++;

    /* generate a maze */
    maze[y][x] = ' ';
    maze_gen_r(y, x);

    /* put start and goal */
    maze[maze_size - 2][maze_size - 2] = 'S';
    maze[1][1] = 'G';
}

/* ---------------------------------------------------------------------
 * SUPPORT FUNCTIONS & MATERIALS FOR THE 3D RENDERER
 * --------------------------------------------------------------------- */

const char mapUUU[4+1] =
    /* 18 */
    "////";  /* 6-8 */

/* is also used as UUUSS */
const char mapUUUS[3][11+1] = {
    /* 0       */
    /* 9       */
    "/////////. ", /* 6 */
    "/////////-:", /* 7 */
    "/////////` "  /* 8 */
    /*      20 */
    /*      29 */
};

const char mapUUUSSS[1+1] =
    /* 0 */
    ":"; /* 7 */
    /* 39 */

const char mapUU[20+1] =
    /* 10 */
    "////////////////////"; /* 4-10 */

const char mapUUS[3][18+1] = {
    /* 0 */
    "//////////-=-     ", /* 4, 10 */
    "//////////=-----  ", /* 5, 9 */
    "//////////--------", /* 6-8 */
    /*             22 */
};

const char mapUUSS[2][3+1] = {
    /* 0 */
    "-  ", /* 5, 9 */
    "---", /* 6-8 */
    /* 37 */
};

const char mapU[36+1] =
    /* 2 */
    "////////////////////////////////////"; /* 1-13 */

const char mapUS[3][10+1] = {
    /* 0 */
    "//==      ", /* 1, 13 */
    "//=====   ", /* 2, 12 */
    "//========", /* 3-11 */
    /*     30 */
};

const char mapS[2][2+1] = {
    /* 0 */
    "= ", /* 0, 14 */
    "==", /* 1-13 */
    /* 38 */
};

void draw_mapUUU()
{
    int i;
    for(i = 0; i < 3; i++)
        draw_trans(mapUUU, 18, i + 6);
}

void draw_mapUUUL()
{
    int i;
    for(i = 0; i < 3; i++)
        draw_trans(mapUUUS[i], 9, i + 6);
}

void draw_mapUUUR()
{
    int i;
    for(i = 0; i < 3; i++)
        draw_mirr_trans(mapUUUS[i], 20, i + 6);
}

void draw_mapUUULL()
{
    int i;
    for(i = 0; i < 3; i++)
        draw_trans(mapUUUS[i == 1 ? 1 : 0], 0, i + 6);
}

void draw_mapUUURR()
{
    int i;
    for(i = 0; i < 3; i++)
        draw_mirr_trans(mapUUUS[i], 29, i + 6);
}

void draw_mapUUULLL()
{
    draw_trans(mapUUUSSS, 0, 7);
}

void draw_mapUUURRR()
{
    draw_mirr_trans(mapUUUSSS, 39, 7);
}

void draw_mapUU()
{
    char i;
    for(i = 0; i < 7; i++)
        draw_trans(mapUU, 10, i + 4);
}

void draw_mapU()
{
    char i;
    for(i = 0; i < 13; i++)
        draw_trans(mapU, 2, i + 1);
}

void draw_mapUUL()
{
    char i;

    draw_trans(mapUUS[0], 0, 4);
    draw_trans(mapUUS[1], 0, 5);

    for(i = 0; i < 3; i++)
        draw_trans(mapUUS[2], 0, i + 6);

    draw_trans(mapUUS[1], 0, 9);
    draw_trans(mapUUS[0], 0, 10);
}

void draw_mapUUR()
{
    char i;

    draw_mirr_trans(mapUUS[0], 22, 4);
    draw_mirr_trans(mapUUS[1], 22, 5);

    for(i = 0; i < 3; i++)
        draw_mirr_trans(mapUUS[2], 22, i + 6);

    draw_mirr_trans(mapUUS[1], 22, 9);
    draw_mirr_trans(mapUUS[0], 22, 10);
}

void draw_mapUULL()
{
    char i;

    draw_trans(mapUUSS[0], 0, 5);

    for(i = 0; i < 3; i++)
        draw_trans(mapUUSS[1], 0, i + 6);

    draw_trans(mapUUSS[0], 0, 9);
}

void draw_mapUURR()
{
    char i;

    draw_mirr_trans(mapUUSS[0], 37, 5);

    for(i = 0; i < 3; i++)
        draw_mirr_trans(mapUUSS[1], 37, i + 6);

    draw_mirr_trans(mapUUSS[0], 37, 9);
}

void draw_mapUL()
{
    char i;

    draw_trans(mapUS[0], 0, 1);
    draw_trans(mapUS[1], 0, 2);

    for(i = 0; i < 9; i++)
        draw_trans(mapUS[2], 0, i + 3);

    draw_trans(mapUS[1], 0, 12);
    draw_trans(mapUS[0], 0, 13);
}

void draw_mapUR()
{
    char i;

    draw_mirr_trans(mapUS[0], 30, 1);
    draw_mirr_trans(mapUS[1], 30, 2);

    for(i = 0; i < 9; i++)
        draw_mirr_trans(mapUS[2], 30, i + 3);

    draw_mirr_trans(mapUS[1], 30, 12);
    draw_mirr_trans(mapUS[0], 30, 13);
}

void draw_mapL()
{
    char i;

    draw_trans(mapS[0], 0, 0);

    for(i = 0; i < 13; i++)
        draw_trans(mapS[1], 0, i + 1);

    draw_trans(mapS[0], 0, 14);
}

void draw_mapR()
{
    char i;

    draw_mirr_trans(mapS[0], 38, 0);

    for(i = 0; i < 13; i++)
        draw_mirr_trans(mapS[1], 38, i + 1);

    draw_mirr_trans(mapS[0], 38, 14);
}

/* ---------------------------------------------------------------------
 * RENDER THE MAZE
 * --------------------------------------------------------------------- */

/* Render the maze in 2D. */
void render_maze_2d()
{
    int i, x_margin = (WIDTH - maze_size) / 2, y_margin = (HEIGHT - maze_size) / 2;

    if(x_margin < 0 || y_margin < 0) return;

    for(i = 0; i < maze_size; i++)
        draw(maze[i], x_margin, y_margin + i);

    draw("P", x_margin + x_pos, y_margin + y_pos);
}

/* Render the maze in 3D. */
void render_maze_3d()
{
    if(MAZE(y_pos-3, x_pos-3) == '*') draw_mapUUULLL();
    if(MAZE(y_pos-3, x_pos+3) == '*') draw_mapUUURRR();
    if(MAZE(y_pos-3, x_pos-2) == '*') draw_mapUUULL();
    if(MAZE(y_pos-3, x_pos+2) == '*') draw_mapUUURR();
    if(MAZE(y_pos-3, x_pos-1) == '*') draw_mapUUUL();
    if(MAZE(y_pos-3, x_pos+1) == '*') draw_mapUUUR();
    if(MAZE(y_pos-3, x_pos  ) == '*') draw_mapUUU();
    if(MAZE(y_pos-2, x_pos-2) == '*') draw_mapUULL();
    if(MAZE(y_pos-2, x_pos+2) == '*') draw_mapUURR();
    if(MAZE(y_pos-2, x_pos-1) == '*') draw_mapUUL();
    if(MAZE(y_pos-2, x_pos+1) == '*') draw_mapUUR();
    if(MAZE(y_pos-2, x_pos  ) == '*') draw_mapUU();
    if(MAZE(y_pos-1, x_pos-1) == '*') draw_mapUL();
    if(MAZE(y_pos-1, x_pos+1) == '*') draw_mapUR();
    if(MAZE(y_pos-1, x_pos  ) == '*') draw_mapU();
    if(MAZE(y_pos  , x_pos-1) == '*') draw_mapL();
    if(MAZE(y_pos  , x_pos+1) == '*') draw_mapR();
}

/* ---------------------------------------------------------------------
 * ROTATE THE MAZE
 * --------------------------------------------------------------------- */

char temp_maze[MAZE_SIZE_MAX][MAZE_SIZE_MAX];

/* Turn right. */
void rotate_maze_left()
{
    int x, y, t;

    for(x = 0; x < maze_size; x++)
        for(y = 0; y < maze_size; y++)
            temp_maze[y][x] = maze[y][x];

    for(x = 0; x < maze_size; x++)
        for(y = 0; y < maze_size; y++)
            maze[y][x] = temp_maze[x][(maze_size - 1) - y];

    t = y_pos;
    y_pos = maze_size - 1 - x_pos;
    x_pos = t;
}

/* Turn left. */
void rotate_maze_right()
{
    int x, y, t;

    for(x = 0; x < maze_size; x++)
        for(y = 0; y < maze_size; y++)
            temp_maze[y][x] = maze[y][x];

    for(x = 0; x < maze_size; x++)
        for(y = 0; y < maze_size; y++)
            maze[y][x] = temp_maze[(maze_size - 1) - x][y];

    t = x_pos;
    x_pos = (maze_size - 1) - y_pos;
    y_pos = t;
}

/* ---------------------------------------------------------------------
 * UI: THE GAME
 * --------------------------------------------------------------------- */

/* If cleared, then return 1, else 0. */
int game()
{
    init_draw_screen();

    maze_gen(counter_get());
    x_pos = maze_size-2;
    y_pos = maze_size-2;
    remaining_steps = 40;

    lcd_setcolor(kowai ? COL_MAGENTA : COL_WHITE);
    render_maze_3d();
    screen_flip();

    while(1)
    {
        keys_update();

        if(INPUT_A) rotate_maze_right();
        else if(INPUT_D) rotate_maze_left();
        else if(INPUT_W && MAZE(y_pos-1, x_pos)!='*') y_pos--, remaining_steps--;
        else if(INPUT_S && MAZE(y_pos+1, x_pos)!='*') y_pos++, remaining_steps--;
        else if(INPUT_K || INPUT_J) remaining_steps = 0;
        else continue;

        lcd_setcolor(kowai ? COL_MAGENTA : COL_WHITE);
        render_maze_3d();
        screen_flip();

        lcd_setcolor(remaining_steps <= 10? COL_RED : COL_MAGENTA);
        lcd_locate(13, 11);
        lcd_printf("%2d steps left", remaining_steps);

        lcd_setcolor(COL_WHITE);

        if(remaining_steps <= 0) return 0;
        if(maze[y_pos][x_pos] == 'G') return 1;
    }
}

/* ---------------------------------------------------------------------
 * UI: GAME RESULT
 * --------------------------------------------------------------------- */

char blank[36+1] =
    /* 2 */
    "                                    ";

void cleared()
{
    char i;
    timer timer = new_timer(100);
    while(remaining_time(timer) > 0);

    /* door-opening effect */
    for(i = 0; i <= 7; i++)
    {
        timer = new_timer(100);
        lcd_locate(2, 7 + i);
        lcd_printf(blank);
        lcd_locate(2, 7 - i);
        lcd_printf(blank);
        while(remaining_time(timer) > 0);
    }

    /* display 2D maze */
    load_screen();
    render_maze_2d();
    screen_flip();

    /* show "cleared" */
    lcd_setcolor(COL_YELLOW);
    lcd_locate(13, 7);
    lcd_printf("c l e a r e d");
    lcd_setcolor(COL_WHITE);

    kowai = 0;
    do keys_update(); while(!INPUT_K && !INPUT_J);
}

void failed()
{
    init_draw_screen();

    /* change color */
    render_maze_3d();
    lcd_setcolor(COL_RED);
    screen_flip();

    /* display 2D maze */
    load_screen();
    render_maze_2d();
    lcd_setcolor(COL_MAGENTA);
    screen_flip();

    /* show "failed" */
    lcd_setcolor(COL_WHITE);
    lcd_locate(14, 7);
    lcd_printf("f a i l e d");

    kowai = 1;
    do keys_update(); while(!INPUT_K && !INPUT_J);
}

/* ---------------------------------------------------------------------
 * UI: TITLE
 * --------------------------------------------------------------------- */

#define LEVEL_MAX ((MAZE_SIZE_MAX - 1) / 2)

char level = 5;

int main()
{
    char sel;

    lcd_ttyopen(1);

  begin:

    sel = 0;

    lcd_cls();
    lcd_setcolor(COL_WHITE);

    lcd_locate(9, 4);
    lcd_printf("m a z e > e s c a p e");
    lcd_locate(14, 8);
    lcd_printf("p l a y");
    lcd_locate(14, 10);
    lcd_printf("l e v e l :");

    /* accents */
    lcd_setcolor(COL_MAGENTA);
    if(kowai)
    {
        lcd_locate(9, 4);
        lcd_printf("      e   e         e");
        lcd_locate(16, 8);
        lcd_printf("r");
        lcd_locate(14, 10);
        lcd_printf(")");
        lcd_locate(22, 10);
        lcd_printf(".");
    }
    else
    {
        lcd_locate(19, 4);
        lcd_printf("e");
    }

    /* cursor */
    lcd_setcolor(COL_MAGENTA);
    lcd_locate(11, 8);
    lcd_printf(">");

    /* level */
    lcd_setcolor(kowai ? COL_MAGENTA : COL_WHITE);
    lcd_locate(26, 10);
    lcd_printf("%2d", level);

    /* main loop */
    while(1)
    {
        keys_update();

        /* up / down */
        if(INPUT_S || INPUT_W)
        {
            sel = !sel;
            lcd_setcolor(COL_MAGENTA);
            lcd_locate(11, 8);
            lcd_printf(!sel ? ">" : " ");
            lcd_locate(11, 10);
            lcd_printf(!sel ? " " : ">");
            lcd_setcolor(COL_WHITE);
        }

        /* left */
        else if(INPUT_A && sel)
        {
            if(level > 1) level--;
            lcd_setcolor(kowai ? COL_MAGENTA : COL_WHITE);
            lcd_locate(26, 10);
            lcd_printf("%2d", level);
            lcd_setcolor(COL_WHITE);
        }

        /* right */
        else if(INPUT_D && sel)
        {
            if(level < LEVEL_MAX - 1) level++;
            lcd_setcolor(kowai ? COL_MAGENTA : COL_WHITE);
            lcd_locate(26, 10);
            lcd_printf("%2d", level);
            lcd_setcolor(COL_WHITE);
        }

        /* decide */
        else if((INPUT_K || INPUT_J) && !sel)
        {
            maze_size = level * 2 + 1;
            if(game()) cleared(); else failed();
            goto begin;
        }
    }

    return 0;
}
