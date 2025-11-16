#include "ncurses.h"
#include "stdint.h"
#include "unistd.h"

int main()
{
    
    uint8_t red = 0;
    uint8_t green = 0;
    uint8_t blue = 0;
    
    initscr();
    start_color();
    curs_set(0);

    // generate colors
    
    WINDOW* color_win = newwin(40,36,0,0);
    init_pair(1, COLOR_WHITE, COLOR_WHITE);
    wrefresh(color_win);
    
    double color_scale = 1000.0 / 255.0;
    int color_num = 8;
    int pair_num = 2;

    int log = 0;

    int inc_val_red = 0x5F;
    for (int red = 0; red < 256; red += inc_val_red) {
	int inc_val_green = 0x5F;
	for (int green = 0; green < 256; green += inc_val_green) {
	    int inc_val_blue = 0x5F;	    
	    for (int blue = 0; blue < 256; blue += inc_val_blue) {

		init_color(color_num,
			   red * color_scale,
			   green * color_scale,
			   blue * color_scale);
		
		init_pair(pair_num, COLOR_BLACK, color_num);	       		
		
		wattron(color_win, COLOR_PAIR(pair_num));
		color_num++;
		pair_num++;
			       
		wprintw(color_win, "%02x%02x%02x", red, green, blue);
		wrefresh(color_win);

		usleep(10000,);
		
		if (blue != 0) {
		    inc_val_blue = 0x28;
		}
	    }
	    
	    if (green != 0) {
		inc_val_green = 0x28;
	    }
	}
	
	if (red != 0) {
	    inc_val_red = 0x28;
	}
    }

    
    // cool blue flicker
    /*
    for (uint8_t red = 0; red < 255; red += inc_val_red) {
	for (uint8_t green = 0; green < 255; green += inc_val_green) {	    
	    for (uint8_t blue = 0; blue < 255; blue += inc_val_blue) {

		init_color(color_num,
			   red * color_scale,
			   green * color_scale,
			   blue * color_scale);

		init_pair(pair_num, color_num, COLOR_BLACK);
		
		wattron(color_win, COLOR_PAIR(pair_num));

		wprintw(color_win, "%02x%02x%02x", red, green, blue);
		wrefresh(color_win);
		usleep(10000);
		
		if (blue != 0) {
		    inc_val_blue = 0x28;
		}
	    }
	    
	    if (green != 0) {
		inc_val_green = 0x28;
	    }
	}
	
	if (red != 0) {
	    inc_val_red = 0x28;
	}
    }
    */

    
    getchar();

    endwin();
    
    return 0;
}
