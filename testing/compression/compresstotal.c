#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>

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

typedef struct __attribute__((packed)) {
    unsigned char blue_val;
    unsigned char green_val;
    unsigned char red_val;
} PIXEL;

typedef struct {
    PIXEL pixel;
    int frequency;
} COLOR_FREQ;

int color_count = 0;
COLOR_FREQ* freq;

//TODO: 
//new idea, read in all colors, then insert into color tables based on
//highest frequnecy first.

int colors_equal(PIXEL a, PIXEL b)
{
    return a.red_val == b.red_val
    && a.green_val == b.green_val
    && a.blue_val == b.blue_val;
}

int locate(COLOR_FREQ cf)
{
    for (int i = 0; i < color_count; i++) {
        if (colors_equal(cf.pixel, freq[i].pixel)) {
            return i;
        }
    }
    return -1;
}


void insert_if_absent(COLOR_FREQ cf)
{
    int i;
    if ((i = locate(cf)) != -1) {
        freq[i].frequency += cf.frequency;
    } else {
        freq[color_count++] = cf;
    }
}

void print_cols();
int main(int argc, char** args)
{
    // read in file and header
    const char* filename = args[1];
    FILE *file = fopen(filename, "r");

    fseek(file, 0, SEEK_END);
    int filesize = ftell(file);
    fseek(file, 0, SEEK_SET);

    uint8_t* tga_bytestream = malloc(filesize);
    fread(tga_bytestream, sizeof(uint8_t), filesize, file);
    
    fclose(file);

    TARGA_HEADER tga_header;
    memcpy(&tga_header, tga_bytestream, sizeof(tga_header));

    printf("HEADER-INFO:\nImage Type:\t%d\nWidth:\t\t%dpx\nHeight:\t\t%dpx\n",
           tga_header.img_type,
           tga_header.width,
           tga_header.height);


    freq = malloc(tga_header.height * tga_header.width * sizeof(COLOR_FREQ));

    int total_pixels = tga_header.height * tga_header.width;
    int curr_pixels = 0;

    // skip past the header to position the cursor where color data begins
    uint8_t* bytestr_cursor = tga_bytestream + sizeof(TARGA_HEADER);

    while (curr_pixels < total_pixels) {
        uint8_t header;
        memcpy(&header, bytestr_cursor++, sizeof(header));
        int type = header >> 7;

        int pixc = 0;
        if (type) { // runlength
            pixc = (header ^ 0x80) + 1;

            PIXEL pixel;
            memcpy(&pixel, bytestr_cursor, sizeof(PIXEL));
            bytestr_cursor += sizeof(pixel) / sizeof(uint8_t);

            insert_if_absent(
                (COLOR_FREQ) {
                    .pixel = pixel,
                    .frequency = pixc
                });

        } else { // raw
            pixc = header+1;
            PIXEL pixel;
            for (int pixel_count = header+1; pixel_count > 0; pixel_count--) {
                memcpy(&pixel, bytestr_cursor, sizeof(PIXEL));
                bytestr_cursor += sizeof(pixel) / sizeof(uint8_t);


                insert_if_absent(
                    (COLOR_FREQ) {
                        .pixel = pixel,
                        .frequency = 1
                    });
            }
        }
        curr_pixels += pixc;
    }

    print_cols();

    return 0;
}

void print_cols()
{
    // init ncurses and print color tables
    initscr();
    start_color();
    curs_set(0);
    WINDOW* palwin = newwin(100,90,1,0);
    //    WINDOW* palwin = newwin(100,84,0,0);    
    refresh();

    init_color(COLOR_BLACK,
               200,
               200,
               200);

    int PAIR_NUM = 2;
    int PIXEL_COLOR = 8;

    double COLORSCALE = 1000.0 / 255.0;

    for (int i = 0; i < color_count; i++) {
        PIXEL p = freq[i].pixel;

        init_color(PIXEL_COLOR,
                   p.red_val * COLORSCALE,
                   p.green_val * COLORSCALE,
                   p.blue_val * COLORSCALE);

        init_pair(PAIR_NUM, COLOR_BLACK, PIXEL_COLOR);

        wattron(palwin, COLOR_PAIR(PAIR_NUM));
        wprintw(palwin, " [%03d,%03d,%03d] ", p.red_val, p.green_val, p.blue_val);
        //        wprintw(palwin, " [%02x,%02x,%02x] ", p.red_val, p.green_val, p.blue_val);
        wattroff(palwin, COLOR_PAIR(PAIR_NUM));

        PAIR_NUM++;
        PIXEL_COLOR++;


        refresh();
        wrefresh(palwin);

        if (PIXEL_COLOR > 256) {
            getchar();
            wclear(palwin);
            PIXEL_COLOR = 8;
            PAIR_NUM = 2;
        }

    }
    getchar();
    endwin();
}
