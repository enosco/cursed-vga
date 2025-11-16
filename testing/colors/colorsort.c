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

    // generate colors
    
    WINDOW* color_win = newwin(56,40,0,0);
//    init_pair(1, COLOR_WHITE, COLOR_WHITE);
    wrefresh(color_win);


    RGB *palette = malloc(256*sizeof(RGB));
    
    double color_scale = 1000.0 / 255.0;
    int color_num = 0;
    int pair_num = 0;

    /*
    int inc_val_red = 0x5F;
    for (int red = 0; red < 256; red += inc_val_red) {
	int inc_val_green = 0x5F;
	for (int green = 0; green < 256; green += inc_val_green) {
	    int inc_val_blue = 0x5F;	    
	    for (int blue = 0; blue < 256; blue += inc_val_blue) {

		palette[color_num++] = (RGB) {
		    .red = red,
		    .green = green,
		    .blue = blue
		};
		
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

    
    for (int grey = 15; grey < 256; grey += 6) {
	palette[color_num++] = (RGB) {
	    .red = grey,
	    .green = grey,
	    .blue = grey
	};
	
    }
    */

    int red_inc = 91;
    for (int red = 0; red < 256; red += red_inc) {
        int green_inc = 93;
        for (int green = 0; green < 256; green += green_inc) {
            int blue_inc = 93;
            for (int blue = 0; blue < 256; blue += blue_inc) {
                palette[color_num++] = (RGB) {
                    .red = red,
                    .green = green,
                    .blue = blue
                };

		if (blue != 0) {  
		    blue_inc = 54;		    
                }

            }
            if (green != 0) {
		green_inc = 27;
            }
        }
        
        if (red != 0) {
	    red_inc = 41;
        }
    }

    for (int i = 30; i <= 240; i += 14) {
	palette[color_num++] = (RGB) {
	    .red = i,
	    .green = i,
	    .blue = i
	};	
    }

//    qsort(palette, 256, sizeof(RGB), compare_rgb);
    RGB rgb;
    for (int i = 0; i < 256; i++) {
	rgb = palette[i];

	
	init_color(i,
		   rgb.red * color_scale,
		   rgb.green * color_scale,
		   rgb.blue * color_scale);
	
	init_pair(i, COLOR_BLACK, i);
	
	wattron(color_win, COLOR_PAIR(i));
	color_num++;
	pair_num++;
	
	wprintw(color_win, " %02x%02x%02x ", rgb.red, rgb.green, rgb.blue);
	wrefresh(color_win);

	wattroff(color_win, COLOR_PAIR(i));
	usleep(100);
    }
    
    getchar();
    endwin();
    
    return 0;
}

int compare_rgb(const void* a, const void* b) {
    RGB* r1 = (RGB*) a;
    RGB* r2 = (RGB*) b;
    
    int r1_rgb_sum = r1->red + r1->green + r1->blue;
    int r2_rgb_sum = r2->red + r2->green + r2->blue;

    return r1_rgb_sum - r2_rgb_sum;    
}
