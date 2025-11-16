#include <locale.h>
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <math.h>
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

typedef struct __attribute__((packed)) {
    unsigned char blue_val;
    unsigned char green_val;
    unsigned char red_val;
} PIXEL;


//int num = 0;
const int COLOR_LIMIT = 216;

PIXEL color_map[COLOR_LIMIT];

int minimum(int a, int b)
{
    return (a < b) ? a : b;
}

int maximum(int a, int b)
{
    return (a > b) ? a : b;
}

float get_hue(uint8_t r, uint8_t g, uint8_t b)
{
    int max_i = maximum(maximum(r,g), b);
   
    float rf,gf,bf;
    rf = (float) r / 255.0;
    gf = (float) g / 255.0;
    bf = (float) b / 255.0;
    
    float max = fmax(fmax(rf, gf), bf);
    float min = fmin(fmin(rf, gf), bf);

    if (max - min == 0) {
        return 0;
    }
    
    float hue = 0.0;
    
    if (max_i == r) {
        hue = (gf - bf) / (max - min);
    } else if (max_i == g) {
        hue = 2.0 + ((bf - rf) / (max - min));
    } else {
        hue = 4.0 + ((rf - gf) / (max - min));
    }

    printf("HUE OF %d, %d, %d: %f\n\n", r, g, b, hue * 60);
    
    return hue * 60;
}

int compare_hue(const void *a, const void *b)
{
    PIXEL* this = (PIXEL*) a;
    PIXEL* other = (PIXEL*) b;

    float this_hue = get_hue(this->red_val, this->green_val, this->blue_val);
    float other_hue = get_hue(other->red_val, other->green_val, other->blue_val);

    return (this_hue < other_hue) - (this_hue > other_hue);
}

int as_hex(PIXEL p)
{
    return (p.red_val << 16) | (p.green_val << 8) | p.blue_val;
}

const int DEFAULT = 1;
int color_is_null(PIXEL p)
{
    return p.red_val == DEFAULT
        && p.green_val == DEFAULT
        && p.blue_val == DEFAULT;
}

// uses hex as hashcode
int locate(PIXEL p)
{
    int index = as_hex(p) % COLOR_LIMIT;
    if (color_is_null(color_map[index]) == 1) {
        return -index;
    } else {
        return index;
    }
}

void insert_if_absent(PIXEL p)
{
    int pos = locate(p);   
    if (pos < 0) {
        color_map[-pos] = p;
    }
}

int main()
{
    const char* filename = "strip1.tga";
//    const char* filename = "redsort.tga";
    FILE *file = fopen(filename, "r");

    TARGA_HEADER tga_header;
    fread(&tga_header, sizeof(char), sizeof(TARGA_HEADER), file);
    
    printf("HEADER-INFO:\nImage Type:\t%d\nWidth:\t\t%dpx\nHeight:\t\t%dpx\n",
           tga_header.img_type,
           tga_header.width,
           tga_header.height);

    
    for (int i = 0; i < COLOR_LIMIT; i++) {
        color_map[i] = (PIXEL) {
            .red_val = DEFAULT,
            .green_val = DEFAULT,
            .blue_val = DEFAULT,
        };
    }      
    
    int limit = tga_header.width * tga_header.height;
    int curr_pix = 0;
    while (curr_pix < limit) {
        uint8_t header;
        fread(&header, sizeof(uint8_t), sizeof(uint8_t), file);

        int type = header >> 7;

        int pixc = 0;
        if (type) {
            pixc = (header ^ 0x80) + 1;
            
            PIXEL pixel;
            fread(&pixel, sizeof(uint8_t), sizeof(PIXEL), file);            
            insert_if_absent(pixel);
        } else {
            pixc = header+1;
            PIXEL pixel;
            for (int pixel_count = header+1; pixel_count > 0; pixel_count--) {
                fread(&pixel, sizeof(uint8_t), sizeof(PIXEL), file);
                insert_if_absent(pixel);
            }
        }
        curr_pix += pixc;
    }
    fclose(file);

    qsort(color_map, COLOR_LIMIT, sizeof(PIXEL), compare_hue);

    
    initscr();
    start_color();
    curs_set(0);
    WINDOW* palwin = newwin(100,90,0,0);
//    WINDOW* palwin = newwin(100,84,0,0);    
    refresh();

    init_color(COLOR_BLACK,
               200,
               200,
               200);
    
    int PAIR_NUM = 2;
    int PIXEL_COLOR = 8;

    double COLORSCALE = 1000.0 / 255.0;
    
    for (int i = 0; i < COLOR_LIMIT; i++) {
        PIXEL p = color_map[i];

        if (color_is_null(p)) {
//            continue;
        }

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
    }
    
    wrefresh(palwin);
    getchar();
    
    endwin();
    
    return 0;
}
