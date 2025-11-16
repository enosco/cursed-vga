#include "ncurses.h"
#include "stdint.h"
#include "unistd.h"
#include "stdlib.h"

typedef struct {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} RGB;

int compare_rgb(const void* a, const void* b);

int main()
{ 
    initscr();
    start_color();
    curs_set(0);

    WINDOW* color_win = newwin(56,64,0,0);
//    init_pair(1, COLOR_WHITE, COLOR_WHITE);

    double color_scale = 1000.0 / 255.0;

    RGB palette[COLORS];
    
    for (int i = 0; i < 256; i++) {
	short r,g,b;
	color_content(i, &r, &g, &b);

//	r /= color_scale;
//	g /= color_scale;
//	b /= color_scale;
	
	wprintw(color_win, " %d %d %d \n", r,g,b);
	palette[i] = (RGB) {
	    .red = r / color_scale,
	    .green = g / color_scale,
	    .blue = b / color_scale
	};
    }

    wrefresh(color_win);    
    getchar();
    // generate colors
    


    for (int i = 0; i < COLORS; i++) {
	RGB color = palette[i];
	
	init_pair(i, COLOR_BLACK, i);
		 
	wattron(color_win, COLOR_PAIR(i));
	wprintw(color_win, " %02x%02x%02x ", color.red, color.green, color.blue);
	wrefresh(color_win);

	wattroff(color_win, COLOR_PAIR(i));
//	usleep(100);
    }

    getchar();
    endwin();
    
    return 0;
}
