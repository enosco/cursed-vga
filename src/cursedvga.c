#include <ncurses.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/time.h>

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

// TODO: Need to make additional structs for different pixel depths

typedef struct __attribute__((packed)) {
    uint8_t blue_val;
    uint8_t green_val;
    uint8_t red_val;
} PIXEL;

typedef struct __attribute__((packed)) {
    int repetition_count;
    PIXEL color_data;
} RUN_LEN_PACKET_DATA;

int get_nearest_color_pair(PIXEL *palette, int red, int green, int blue, WINDOW* log_win) {

    // TODO: FIXME FIXME FIXME FIXME      
    // index in the color palette array corresponds to its pair num
    /*
    wprintw(log_win, "Finding nearest color for r:%d g:%d b:%d\n", red, green, blue);  
    
    int nearest_pair = 0;
    int red_index = 0;

    if (red < (95 >> 1)) {
	nearest_red = 0;
    } else {
	for (; red_index < 240; red_index += 40) {
	    int existing_red = palette[red_index].red_val;
	    wprintw(log_win,"Testing against red val: %d\n", existing_red);
	    
	    if (red <= existing_red) {
		nearest_red = existing_red;
		break;
	    }	
	}
    }
	
    wprintw(log_win, "Nearest red located at index: %d, value: %d\n", red_index, nearest_red);
    
    int green_index = red_index;

    if (green < (95 >> 1)) {
	nearest_green = 0;
    } else {   
	for (; green_index < 240; green_index += 5) {
	    int existing_green = palette[green_index].green_val;
	    wprintw(log_win, "Testing against green val: %d\n", existing_green);
	    
	    if (green <= existing_green) {
		nearest_green = existing_green;
		break;
	    }
	}
    }
    
//    printw("Nearest green located at index: %d, value: %d\n", green_index, nearest_green);    
    
    int blue_index = green_index;
    if (blue < (95 >> 1)) {
	nearest_blue = 0;
    } else {   
	for (; blue_index < 240; blue_index++) {
	    int existing_blue = palette[blue_index].blue_val;
	    wprintw(log_win,"Testing against blue val: %d\n", existing_blue);
	    
	    if (blue <= existing_blue) {
		nearest_blue = existing_blue;
		break;
	    }
	}
    }
    
//    printw("Nearest blue located at index: %d, value: %d\n", blue_index, nearest_blue);
    wprintw(log_win,"Nearest color at index: %d, r: %d, g: %d, b: %d\n", blue_index, nearest_red, nearest_green, nearest_blue);
    // TODO: Make this intelligible, please
//    refresh();
    return blue_index;
    */

    const double COLORSCALE = 255.0 / 1000.0;
    
    for (int pair_num = 1; pair_num < 64; pair_num++) {
	short fg, bg, r, g, b;
	pair_content(pair_num, &fg, &bg);
	color_content(bg, &r, &g, &b);

	r = r * COLORSCALE;
	g = g * COLORSCALE;
	b = b * COLORSCALE;
	
	if (red <= r) {
	    refresh();
	    return pair_num;
	}
    }
    return -1;    
}

void generate_palette(PIXEL *palette) {
    int color_num = 8;
    int pair_num = 1;
    const double COLORSCALE = 1000.0 / 255.0;      

    














    
/*
    const float red_weight = 6;
    const float green_weight = 8;
    const float blue_weight = 5;
    
    const float initial_increment = 95;
    
    float red_inc = 95;
    for (float red = 0; red < 256; red += red_inc) {
        float green_inc = 95;
        for (float green = 0; green < 256; green += green_inc) {
            float blue_inc = 90;
            for (float blue = 0; blue < 256; blue += blue_inc) {
                palette[colors_idx++] = (PIXEL) {
                    .red_val = red,
                    .green_val = green,
                    .blue_val = blue
                };

		init_color(color_num,
			   red * COLORSCALE,
			   green * COLORSCALE,
			   blue * COLORSCALE);
	    
		init_pair(pair_num, COLOR_BLACK, color_num);				

		color_num++;
		pair_num++;

//		printw("B:%f\n", blue);
                if (blue != 0 && blue_inc != 55) {
		    // TODO: error here, goes to 0xfe instead of 0xff
		    // this is a flawed algorithm
		    
		    blue_inc = (255 - blue_inc) / (blue_weight-2);
//		    printw("%f\n", blue_inc);
                }

            }
            if (green != 0) {
                green_inc = (255 - green_inc) / (green_weight-2);
            }
        }
        
        if (red != 0) {
            red_inc = (255 - red_inc) / (red_weight-2);
        }
    }
*/
    // grayscale first, to keep things simple
    for (int grey_val = 0; grey_val < 256; grey_val += 5) {
        palette[pair_num] = (PIXEL) {
            .red_val = grey_val,
            .green_val = grey_val,
            .blue_val = grey_val
        };

	init_color(color_num,
		   grey_val * COLORSCALE,
		   grey_val * COLORSCALE,
		   grey_val * COLORSCALE);
	
	init_pair(pair_num, COLOR_BLACK, color_num);
	color_num++;
	pair_num++;		
    }    

    for (int i = 1; i < 64; i++) {
	attron(COLOR_PAIR(i));
	printw("%02x%02x%02x ", palette[i].red_val, palette[i].green_val, palette[i].blue_val);
	refresh();
	attroff(COLOR_PAIR(i));
    }

    getchar();
    
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
	printf("ERR: No Filename Specified\n");
	return 1;
    }
    
    const char* filename = argv[1];
    
    FILE *file = fopen(filename, "r");

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

    if (tga_header.height * tga_header.width > 256) {
	printf("WARNING: Images containing >256 pixels are currently unsupported, "
	       "displayed image may be mangled!!\n");
    }
    
    printf("Press enter to continue...");
    getchar();

    initscr();
    curs_set(0);
    start_color();
    
    int PAIR_NUM = 1;
    int PIXEL_COLOR = 8; // start at 8 so default colors are not overwritten
    
    init_color(PIXEL_COLOR, 1000,1000,1000);
    init_pair(PAIR_NUM, COLOR_WHITE, PIXEL_COLOR);
    refresh();
    
    WINDOW *img_win = newwin(tga_header.height, tga_header.width << 1, 0, 0);
    WINDOW *log_win = newwin(tga_header.height, 50, 0, (tga_header.width << 1) + 1);
    scrollok(log_win, 1);
    
    wbkgd(img_win, ' ' | A_REVERSE);

    const double COLORSCALE = 1000.0 / 255.0;
    const int WAIT_TIME_USEC = 1280000 / (tga_header.height * tga_header.width);
//    const int WAIT_TIME_USEC = 128000000 / (tga_header.height * tga_header.width);
    
    int x_pos = 0;
    int y_pos = tga_header.height-1;

    int log = 0;

    // generate fixed 256-color palette
    PIXEL palette[256];
    generate_palette(palette);
            
    // begin parsing
    while (y_pos >= 0) {	
	// read in header (1 byte)
	uint8_t header;
	fread(&header, sizeof(uint8_t), sizeof(uint8_t), file);

	if (header >> 7) { // if true, found a run-length packet

	    wprintw(log_win, "Found Run-Length Packet, Rep. Count: %d\n", (header ^ 0x80) + 1);
	    wrefresh(log_win);
	    PIXEL pixel_data;
	    
	    fread(&pixel_data, sizeof(uint8_t), sizeof(PIXEL), file);

	    // repetition count is the header's 7 rightmost  bits plus 1
	    for (int rep_c = (header ^ 0x80) + 1; rep_c > 0; rep_c--) {
		wprintw(log_win, "Pixels Left: %d\n", rep_c);
		wrefresh(log_win);

		int pair_num = get_nearest_color_pair(palette,
						      pixel_data.red_val,
						      pixel_data.green_val,
						      pixel_data.blue_val,
						      log_win);
		
			    
		mvwchgat(img_win, y_pos, x_pos, 2, 0, pair_num, NULL);
		x_pos+=2;
		
		// move to next row if x_pos exceeds image width
		if (x_pos >= (tga_header.width << 1)) {
		    x_pos = 0;
		    y_pos--;		
		}

		wrefresh(img_win);
		usleep(WAIT_TIME_USEC);
	    }
	    
	    PAIR_NUM++;
	    PIXEL_COLOR++;	  

	} else { // otherwise, raw rgb packet
	    wprintw(log_win, "Found Raw RGB Packet, Rep. Count: %d\n", (header + 1));
	    wrefresh(log_win);

	    int pixels_in_packet = header + 1;

	    PIXEL pixel_data;
	    for (; pixels_in_packet > 0; pixels_in_packet--) {
		fread(&pixel_data, sizeof(uint8_t), sizeof(PIXEL), file);

		int pair_num = get_nearest_color_pair(palette,
						      pixel_data.red_val,
						      pixel_data.green_val,
						      pixel_data.blue_val,
						      log_win);
			    
		mvwchgat(img_win, y_pos, x_pos, 2, 0, PAIR_NUM, NULL);
		x_pos+=2;
		
		// move to next row if x_pos exceeds image width
		if (x_pos >= (tga_header.width << 1)) {
		    x_pos = 0;
		    y_pos--;		
		}

		wrefresh(img_win);
		usleep(WAIT_TIME_USEC);
	    }	   	    
	}	
    }

    fclose(file);
    
    getchar();
    
    endwin();

    return 0;
}
