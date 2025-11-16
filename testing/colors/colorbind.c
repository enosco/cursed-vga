#include "ncurses.h"
#include "stdint.h"
#include "unistd.h"

typedef struct {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} PIXEL;

int get_nearest_color(PIXEL *palette, int red, int green, int blue) {
    // ignore grayscale for now
    int nearest_red, nearest_green, nearest_blue = 0;

    // index in the color palette array corresponds to its pair num
    printw("Finding nearest color for r:%d g:%d b:%d\n", red, green, blue);
    //
    
    int nearest_pair = 0;
    int red_index = 0;

    if (red < (95 >> 2)) {
	nearest_red = 0;
    } else {
	for (; red_index < 240; red_index += 40) {
	    int existing_red = palette[red_index].red;
	    printw("Testing against red val: %d\n", existing_red);
	    
	    if (red <= existing_red) {
		nearest_red = palette[red_index].red;
		break;
	    }	
	}
    }
	
    printw("Nearest red located at index: %d, value: %d\n", red_index, nearest_red);
    
    int green_index = red_index;

    if (green < (95 >> 2)) {
	nearest_green = 0;
    } else {   
	for (; green_index < 240; green_index += 5) {
	    int existing_green = palette[green_index].green;
	    printw("Testing against green val: %d\n", existing_green);
	    
	    if (green <= existing_green) {
		nearest_green = palette[green_index].green;
		break;
	    }
	}
    }
    
    printw("Nearest green located at index: %d, value: %d\n", green_index, nearest_green);    
    
    int blue_index = green_index;
    if (blue < (95 >> 2)) {
	nearest_blue = 0;
    } else {   
	for (; blue_index < 240; blue_index++) {
	    int existing_blue = palette[blue_index].blue;
	    printw("Testing against blue val: %d\n", existing_blue);
	    
	    if (blue <= existing_blue) {
		nearest_blue = existing_blue;
		break;
	    }
	}
    }

    
    printw("Nearest blue located at index: %d, value: %d\n", blue_index, nearest_blue);
    printw("Nearest color: r: %d, g: %d, b: %d\n", nearest_red, nearest_green, nearest_blue);
    refresh();
    return -1;
}

int main() {
    PIXEL colors[256];
    // fill first 16 with grey
    int colors_idx = 0;

    const float red_weight = 6;
    const float green_weight = 8;
    const float blue_weight = 5;
    
    const float initial_increment = 95;
    
    float red_inc = 95;
    for (float red = 0; red < 256; red += red_inc) {
	float green_inc = 95;
	for (float green = 0; green < 256; green += green_inc) {
	    float blue_inc = 95;
	    for (float blue = 0; blue < 256; blue += blue_inc) {
		colors[colors_idx++] = (PIXEL) {
		    .red = red,
		    .green = green,
		    .blue = blue
		};
		if (blue != 0) {
		    blue_inc = (255 - initial_increment) / (blue_weight-2);
		}

	    }
	    if (green != 0) {
		green_inc = (255 - initial_increment) / (green_weight-2);
	    }
	}
	
	if (red != 0) {
	    red_inc = (255 - initial_increment) / (red_weight-2);
	}
    }
    
    for (int grey_val = 8; grey_val < 255; grey_val += 16) {
	colors[colors_idx++] = (PIXEL) {
	    .red = grey_val,
	    .green = grey_val,
	    .blue = grey_val
	};
    }
      
    // display color palette
    initscr();
    start_color();
    curs_set(0);
    
    WINDOW* color_win = newwin(100, 30, 0,0);
    init_pair(1, COLOR_WHITE, COLOR_WHITE);
    wrefresh(color_win);
    /*
    double color_scale = 1000.0 / 255.0;
    int color_num = 0;
    int pair_num = 0;

    for (int i = 0; i < 256; i++) {
	PIXEL p = colors[i];
	
	init_color(color_num,
		   p.red * color_scale,
		   p.green * color_scale,
		   p.blue * color_scale);
	
	init_pair(pair_num, COLOR_BLACK, color_num);	       		
	
	wattron(color_win, COLOR_PAIR(pair_num));
	color_num++;
	pair_num++;
	
	wprintw(color_win, "%02x%02x%02x", p.red, p.green, p.blue);
	wrefresh(color_win);
	
	usleep(1000);
    }
    */
    get_nearest_color(colors, 255, 0, 127);
    getchar();

    endwin();   
    return 0;
}
