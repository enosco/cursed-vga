#include <ncurses.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/time.h>

#define CHARS_PER_PIXEL 2
#define COLORSCALE (1000.0 / 255.0)

typedef struct __attribute__((packed)) {
    char   id_len;
    char   color_map_type;
    char   image_type;
    short  color_map_origin;
    short  color_map_len;
    char   color_map_entry_size;
    short  x_orig;
    short  y_orig;
    short  width;
    short  height;
    char   pixel_depth;
    char   img_desc_byte;
} TARGA_HEADER;

typedef struct __attribute__((packed)) {
    uint8_t blue_val;
    uint8_t green_val;
    uint8_t red_val;
} PIXEL;

typedef struct {
    enum {RUNLEN, RAW} type;
    int count; // either repetition count or number of pixels in packet depending on type
} PACKET_INFO;

typedef struct __attribute__((packed)) {
    int repetition_count;
    PIXEL color_data;
} RUN_LEN_PACKET_DATA;

int is_gray(int r, int g, int b) {
    return -1;
}

int get_nearest_color_pair(PIXEL *palette, int red, int green, int blue, WINDOW* log_win) {
    int palette_index = 0;
    int ceil, floor = 0;
    
//    int red_ceil, red_floor = 0;
    do {
	floor = palette[palette_index].red_val;	
	ceil = palette[palette_index + 40].red_val;
	palette_index += 40; // 40 cells per table
    } while (ceil < red && palette_index < (5 * 40));
    
//    int ceil_diff = red_ceil - red;
//    int floor_diff = red - red_floor;
    
//    palette_index = (floor_diff < ceil_diff) ? palette_index - 40 : palette_index;
    palette_index = ((ceil - red) < (red - floor))
		      ? palette_index - 40
		      : palette_index;

    ceil = 0;
    floor = 0;    
    do {
	floor = palette[palette_index].green_val;	
	ceil = palette[palette_index + 6].green_val;
	palette_index += 5; // 5 cells per row
    } while (ceil < green && palette_index < (5 * 40));

    palette_index = ((ceil - green) < (green - floor))
		      ? palette_index - 6
		      : palette_index;

    ceil = 0;
    floor = 0;    
    do {
	floor = palette[palette_index].blue_val;
	ceil = palette[palette_index + 1].blue_val;
	palette_index++;; 
    } while (ceil < blue && palette_index < (5 * 40));

    palette_index = ((ceil - blue) < (blue - floor))
		      ? palette_index - 1
		      : palette_index;

		      
    /*
    printf("ceil: %d, floor: %d, for red val: %d\n\r", red_ceil, red_floor, red);
    printf("ceil diff: %d, floor diff: %d\n\r", ceil_diff, floor_diff);
    printf("closest: %d at idx %d", palette[palette_index].red_val, palette_index);
    */
        
    return palette_index;
}

int compare_rgb(const void* a, const void* b) {
    PIXEL* p1 = (PIXEL*) a;
    PIXEL* p2 = (PIXEL*) b;
    
    int red_diff = p1->red_val - p2->red_val;
    if (red_diff != 0) {
	return red_diff;
    }
    
    int green_diff = p1->green_val - p2->green_val;
    if (green_diff != 0) {
	return green_diff;
    }

    return p1->blue_val - p2->blue_val;
}

void generate_palette(PIXEL *palette) {
    int color_num = 0;
    int pair_num = 0;
        
    int red_inc = 91;
    for (int red = 0; red < 256; red += red_inc) {
        int green_inc = 93;
        for (int green = 0; green < 256; green += green_inc) {
            int blue_inc = 93;
            for (int blue = 0; blue < 256; blue += blue_inc) {
                palette[color_num++] = (PIXEL) {
                    .red_val = red,
                    .green_val = green,
                    .blue_val = blue
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

    for (int i = 15; i <= 240; i += 15) {
	palette[color_num++] = (PIXEL) {
	    .red_val = i,
	    .green_val = i,
	    .blue_val = i
	};	
    }

    //qsort(palette, 256, sizeof(PIXEL), compare_rgb);
    
    int c = 0;
    for (int i = 0; i < 256; i++) {
	init_color(i,
		   palette[i].red_val * COLORSCALE,
		   palette[i].green_val * COLORSCALE,
		   palette[i].blue_val * COLORSCALE);	          
	
	init_pair(i, COLOR_BLACK, i);

	attron(COLOR_PAIR(i));
	printw("%02x%02x%02x ", palette[i].red_val, palette[i].green_val, palette[i].blue_val);
//	printw("     ", palette[i].red_val, palette[i].green_val, palette[i].blue_val);
	
	refresh();
	attroff(COLOR_PAIR(i));

	c++;
	if (c == 5) {
	    printw("\n");
	    c = 0;
	}
    }

    getchar();
    erase();
    refresh();
}

void setup_ncurses(WINDOW* win, int height, int width) {

}

// Initializes color with given rgb values and
// returns corresponding color pair
int initialize_color(int r, int g, int b) {
    static int color_pair_num = 2;
    static int color_num = 8;
    
    if (color_num > COLORS) {
	return -1;
    }

    if (color_pair_num > COLOR_PAIRS) {
	return -2;
    }
    
    init_color(color_num,
	       r * COLORSCALE,
	       g * COLORSCALE,
	       b * COLORSCALE);
    
    init_pair(color_pair_num, COLOR_BLACK, color_num);

    int pair_to_return = color_pair_num;
    color_num++;
    color_pair_num++;

    return pair_to_return;
}

/* changes the next attributes of next 2 pixels to
 * color at color_pair and updates the cursor
 *
 * returns true once the last pixel of the image has been placed
 */
int set_color_and_update_cursor(WINDOW* img_win, int color_pair, int pixels_affected) {
    int x_pos, y_pos, x_max, y_max;
    getyx(img_win, y_pos, x_pos);
    getmaxyx(img_win, y_max, x_max);

    int cells_affected = pixels_affected * CHARS_PER_PIXEL;

    // TODO: might need to handle case where run-length packet
    // overflows across line
    wchgat(img_win, cells_affected, 0, color_pair, NULL);
    usleep(10000);

    // adjust cursor and move to next row if x_pos exceeds image width    
    x_pos += cells_affected;
    if (x_pos >= x_max) {
	x_pos = 0;
	y_pos--;		
    }

    if (y_pos < 0) {
	return true;
    } else {           
	wmove(img_win, y_pos, x_pos);
	return false;
    }
    // FIXME: need to adjust return value so that the loop stops correctly
}

int display_run_length_packet(WINDOW* img_win, FILE* file, int repetition_count, PIXEL* palette) {
    PIXEL pixel;
    fread(&pixel, sizeof(uint8_t), sizeof(PIXEL), file);    
    int pair_num = get_nearest_color_pair(palette,
					  pixel.red_val,
					  pixel.green_val,
					  pixel.blue_val,
					  NULL);

    int8_t ret = set_color_and_update_cursor(img_win, pair_num, repetition_count);
    wrefresh(img_win);
    return ret;
}

int display_raw_rgb_packet(WINDOW* img_win, FILE *file, int pixel_count, PIXEL* palette) {
    int ret = 0;
    for (; pixel_count > 0; pixel_count--) {
	PIXEL pixel;
	fread(&pixel, sizeof(uint8_t), sizeof(PIXEL), file);

	int pair_num = get_nearest_color_pair(palette,
					  pixel.red_val,
					  pixel.green_val,
					  pixel.blue_val,
					  NULL);

	ret = set_color_and_update_cursor(img_win, pair_num, 1);
	usleep(5000);
	wrefresh(img_win);	
    }
    return ret;
}

int display_image_data(WINDOW* img_win, FILE* file, PACKET_INFO packet_info, PIXEL* palette) {
    switch (packet_info.type) {
	case RUNLEN:
	    return display_run_length_packet(img_win, file, packet_info.count, palette);
	case RAW:
	    return display_raw_rgb_packet(img_win, file, packet_info.count, palette);
    }
}

int main(int argc, char* argv[]) {    
    if (argc < 2) {
	printf("ERR: No Filename Specified\n");
	return 1;
    }
    
    const char* filename = argv[1];
    
    FILE* file = fopen(filename, "r");

    /***** Startup Functionality *****/
    
    TARGA_HEADER tga_header;
    fread(&tga_header, sizeof(char), sizeof(TARGA_HEADER), file);
    
    printf("Image Name:\t%s\nWidth:\t\t%dpx\nHeight:\t\t%dpx\n"
	   "Image Type:\t%d\nPixel Depth:\t%d-bit\n",
	   filename,
	   tga_header.width,
	   tga_header.height,
	   tga_header.image_type,
	   tga_header.pixel_depth);

    if (tga_header.image_type != 10) {
	printf("WARNING: Image Type of %d is currently unsupported, "
	       "displayed image may be mangled!!\n", tga_header.image_type);
    }

    if (tga_header.color_map_type != 0) {
	printf("WARNING: Color-mapped images are currently unsupported, "
	       "displayed image may be mangled!!\n");
    }

    if (tga_header.x_orig != 0 || tga_header.y_orig != 0) {
	printf("WARNING: Non-zero x/y origins are currently unsupported, "
	       "displayed image may be mangled!!\n");
    }

    if (tga_header.height > 256) {
	printf("WARNING: Image Height is >256 pixels,"
	       "your terminal might not be able to display the entire image!\n");
    }

    printf("Press enter to continue...");
    getchar();

    /***** Initialize NCURSES *****/
    
    initscr();
    curs_set(0);
    start_color();

    // generate fixed 256-color palette
    PIXEL palette[256];
    generate_palette(palette);

//    get_nearest_color_pair(palette, 2, 0, 0, NULL);
    getchar();
    
    init_pair(0, 0, 0);
    attron(COLOR_PAIR(1));
    refresh();
    
    WINDOW *img_win = newwin(tga_header.height, tga_header.width << 1, 0, 0);
    WINDOW *log_win = newwin(tga_header.height, 50, 0, (tga_header.width << 1) + 1);

    scrollok(log_win, 1);
    
    wbkgd(img_win, ' ' | A_REVERSE);
    wrefresh(img_win);

    /***** Begin Output *****/
    
    // image data begins in the bottom left corner, move the cursor to match
    wmove(img_win, tga_header.height-1, 0);
       
    int8_t finished_flag = 0;
    while (!finished_flag) {
	uint8_t header_byte;
	fread(&header_byte, sizeof(uint8_t), sizeof(uint8_t), file);

	PACKET_INFO pkt_info;	
	
	if (header_byte >> 7) {
	    wprintw(log_win, "Found Run-Length Packet, Rep. Count: %d\n",
		    (header_byte ^ 0x80) + 1);
	    
	    wrefresh(log_win);
	    pkt_info = (PACKET_INFO) {
		.type = RUNLEN,
		.count = (header_byte ^ 0x80) + 1
	    };      
	} else {
	    wprintw(log_win, "Found Raw RGB Packet, Pixel Count: %d\n", (header_byte + 1));
	    wrefresh(log_win);

	    pkt_info = (PACKET_INFO) {
		.type = RAW,
		.count = header_byte + 1
	    };
	}

	finished_flag = display_image_data(img_win, file, pkt_info, palette);
    }
    
    fclose(file);
    
    getchar();
    
    endwin();

    return 0;
}
