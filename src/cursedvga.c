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
    unsigned char header;
    unsigned char blue_val;
    unsigned char green_val;
    unsigned char red_val;
} PIXEL_PACKET;


typedef struct __attribute__((packed)) {
    uint8_t blue_val;
    uint8_t green_val;
    uint8_t red_val;
} PIXEL;

typedef struct __attribute__((packed)) {
    int repetition_count;
    PIXEL color_data;
} RUN_LEN_PACKET_DATA;

int main(int argc, char* argv[]) {
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
    
    int x_pos = 0;
    int y_pos = tga_header.height-1;

    int log = 0;
 
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

		init_color(PIXEL_COLOR,
			   pixel_data.red_val * COLORSCALE,
			   pixel_data.green_val * COLORSCALE,
			   pixel_data.blue_val * COLORSCALE);
	    
		init_pair(PAIR_NUM, COLOR_BLACK, PIXEL_COLOR);
		
			    
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
	    
	    PAIR_NUM++;
	    PIXEL_COLOR++;	  

	} else { // otherwise, raw rgb packet
	    wprintw(log_win, "Found Raw RGB Packet, Rep. Count: %d\n", (header + 1));
	    wrefresh(log_win);

	    int pixels_in_packet = header + 1;

	    PIXEL pixel_data;
	    for (; pixels_in_packet > 0; pixels_in_packet--) {
		fread(&pixel_data, sizeof(uint8_t), sizeof(PIXEL), file);

		init_color(PIXEL_COLOR,
			   pixel_data.red_val * COLORSCALE,
			   pixel_data.green_val * COLORSCALE,
			   pixel_data.blue_val * COLORSCALE);
	    
		init_pair(PAIR_NUM, COLOR_BLACK, PIXEL_COLOR);
		
			    
		mvwchgat(img_win, y_pos, x_pos, 2, 0, PAIR_NUM, NULL);
		x_pos+=2;
		
		// move to next row if x_pos exceeds image width
		if (x_pos >= (tga_header.width << 1)) {
		    x_pos = 0;
		    y_pos--;		
		}

		wrefresh(img_win);
		usleep(WAIT_TIME_USEC);
	       
		PAIR_NUM++;
		PIXEL_COLOR++;	  
	    }	   	    
	}
	
	if (PAIR_NUM > 255) {
	    wprintw(log_win, "ERR: COLOR_PAIR OVERFLOW");
	    wrefresh(log_win);	   
	    break;
	}
		
	if (PIXEL_COLOR > 255) {
	    wprintw(log_win, "ERR: PIXEL COLOR OVERFLOW");
	    wrefresh(log_win);
	    break;
	}			
    }

    fclose(file);
    
    getchar();
    
    endwin();

    return 0;
}
