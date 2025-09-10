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
///    wchgat(img_win, CHARS_PER_PIXEL, 0, color_pair, NULL);

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

int display_run_length_packet(WINDOW* img_win, FILE* file, int repetition_count) {
    PIXEL pixel;
    fread(&pixel, sizeof(uint8_t), sizeof(PIXEL), file);    
    int pair_num = initialize_color(pixel.red_val,
				    pixel.green_val,
				    pixel.blue_val);

    int8_t ret = set_color_and_update_cursor(img_win, pair_num, repetition_count);
    wrefresh(img_win);
    return ret;
}

int display_raw_rgb_packet(WINDOW* img_win, FILE *file, int pixel_count) {
    int ret = 0;
    for (; pixel_count > 0; pixel_count--) {
	PIXEL pixel;
	fread(&pixel, sizeof(uint8_t), sizeof(PIXEL), file);

	int pair_num = initialize_color(pixel.red_val,
					pixel.green_val,
					pixel.blue_val);
	
	ret = set_color_and_update_cursor(img_win, pair_num, 1);
	usleep(5000);
	wrefresh(img_win);	
    }
    return ret;
}

int display_image_data(WINDOW* img_win, FILE* file, PACKET_INFO packet_info) {
    switch (packet_info.type) {
	case RUNLEN:
	    return display_run_length_packet(img_win, file, packet_info.count);
	case RAW:
	    return display_raw_rgb_packet(img_win, file, packet_info.count);
    }
}


int main(int argc, char* argv[]) {
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

    if (tga_header.height * tga_header.width > 256) {
	printf("WARNING: Images containing >256 pixels are currently unsupported, "
	       "displayed image may be mangled!!\n");
    }
    
    printf("Press enter to continue...");
    getchar();


    /***** Initialize NCURSES *****/
    
    initscr();
    curs_set(0);
    start_color();

    init_pair(1, COLOR_WHITE, COLOR_WHITE);
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

	finished_flag = display_image_data(img_win, file, pkt_info);
    }
    
    fclose(file);
    
    getchar();
    
    endwin();

    return 0;
}
