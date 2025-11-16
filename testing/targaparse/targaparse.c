#include <locale.h>
#include <ncurses.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>

// char - 1 byte
// short - 2 bytes
// int - 4 bytes
// long - 8 bytes

typedef struct __attribute__((packed)) {
    char   id_len;
    char   color_map_type;
    char   img_type;
    short  color_map_origin;
    short  color_map_len;
    char   color_map_entry_size;
    short  x_orig;
    short  y_orig;
    short  width;
    short  height;
    char   pix_depth;
    char   img_desc_byte;
} TARGA_HEADER;

typedef struct __attribute__((packed)) {
    unsigned char header;
    unsigned char blue_val;
    unsigned char green_val;
    unsigned char red_val;
} PIXEL_PACKET;

int main() {
    const char* filename = "bomp.tga";
    FILE *file = fopen(filename, "r");

    TARGA_HEADER tga_header;
    fread(&tga_header, sizeof(char), sizeof(TARGA_HEADER), file);

    printf("HEADER-INFO:\nImage Type:\t%d\nWidth:\t\t%dpx\nHeight:\t\t%dpx\n", tga_header.img_type, tga_header.width, tga_header.height);
    printf("Press enter to continue...");
    getchar();

    initscr();
    curs_set(0);
    start_color();
    
    int PAIR_NUM = 1;
    int PIXEL_COLOR = 1;
    
    init_color(PIXEL_COLOR, 1000,1000,1000);
    init_pair(PAIR_NUM, COLOR_WHITE, PIXEL_COLOR);
    refresh();
    WINDOW *img_win = newwin(tga_header.height, tga_header.width << 1, 0, 0);

    wbkgd(img_win, ' ' | A_REVERSE);
    werase(img_win);
//    wrefresh(img_win);
    double COLORSCALE = 1000.0 / 255.0;

    // Use pixel-by-pixel algorithm first,
    // just in case any of the packets run over the line

    int x_pos = 0;
    int y_pos = tga_header.height-1;

    int log = 0;
    while (y_pos >= 0) {

	PIXEL_PACKET packet;
	fread(&packet, sizeof(char), sizeof(PIXEL_PACKET), file);
	    
	init_color(PIXEL_COLOR,
		   packet.red_val * COLORSCALE,
		   packet.green_val * COLORSCALE,
		   packet.blue_val * COLORSCALE);
	
	init_pair(PAIR_NUM, COLOR_BLACK, PIXEL_COLOR);



	int pixels_in_packet = (packet.header ^ 0x80) + 1;
	mvprintw(log++, 30, "PACKET HEADER: %u\n", packet.header);
	mvprintw(log++, 30, "PIXELS IN PACKET: %u", pixels_in_packet);
	refresh();

	refresh();
	for (;pixels_in_packet > 0; pixels_in_packet--) {
	    mvwchgat(img_win, y_pos, x_pos, 2, 0, PAIR_NUM, NULL);

	    x_pos+=2;
	    if (x_pos >= (tga_header.width << 1)) {
		x_pos = 0;
		y_pos--;		
	    }

	    
	    wrefresh(img_win);	
	    usleep(50000);
	}
	

	if (PAIR_NUM > 255) {
	    PAIR_NUM = 1;
	    mvprintw(0, 50, "PAIR NUM OVERFLOW");
	    refresh();
	}
	
	if (PIXEL_COLOR > 255) {
	    PIXEL_COLOR = 1;
	    mvprintw(0, 50, "PIX_COLOR OVERFLOW");
	    refresh();
	}
	
	PAIR_NUM++;
	PIXEL_COLOR++;

    }

    /*
    int y_pos = tga_header.height-1;
    while (y_pos >= 0) {
	int x_pos = 0;
	while (x_pos < tga_header.width << 1) {
	   
	    PIXEL_PACKET packet;
	    fread(&packet, sizeof(char), sizeof(PIXEL_PACKET), file);
	    
	    init_color(PIXEL_COLOR,
		       packet.red_val * COLORSCALE,
		       packet.green_val * COLORSCALE,
		       packet.blue_val * COLORSCALE);

	    init_pair(PAIR_NUM, COLOR_BLACK, PIXEL_COLOR);

	    
	    mvwchgat(img_win, y_pos, x_pos, 2, 0, PAIR_NUM, NULL);

	    PAIR_NUM++;
	    PIXEL_COLOR++;
	    usleep(5000);
	    wrefresh(img_win);

	    
	    if (PAIR_NUM > 255) {
		PAIR_NUM = 1;
	    }

	    if (PIXEL_COLOR > 255) {
		PIXEL_COLOR = 1;
	    }
	    
	    x_pos+=2;
	}

	y_pos--;
    }
    */
    fclose(file);
    
    wrefresh(img_win);
    while(1) {}
    endwin();

    return 0;
}
