#include <locale.h>
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
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

typedef struct {
    unsigned char blue_val;
    unsigned char green_val;
    unsigned char red_val;
    int count;
} COLOR_COUNT;

typedef struct __attribute__((packed)) {
    unsigned char blue_val;
    unsigned char green_val;
    unsigned char red_val;
} PIXEL;

int num = 0;
const int COLOR_LIMIT = 32;
int BOUND = 0;
//COLOR_COUNT cc_arr[COLOR_LIMIT];
COLOR_COUNT* cc_arr;

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
    
    float hue = 0.0;

    if (max_i == r) {
        hue = (gf - bf) / (max - min);
    } else if (max_i == g) {
        hue = 2.0 + ((bf - rf) / (max - min));
    } else {
        hue = 4.0 + ((rf - gf) / (max - min));
    }

    return hue * 60;
}

int compare_hue_and_lightness(const void *a, const void *b)
{
    COLOR_COUNT* this = (COLOR_COUNT*) a;
    COLOR_COUNT* other = (COLOR_COUNT*) b;

    float this_hue = get_hue(this->red_val, this->green_val, this->blue_val);
    float other_hue = get_hue(other->red_val, other->green_val, other->blue_val);

    return (this_hue < other_hue) - (this_hue > other_hue);
}

int compare_hue(const void *a, const void *b)
{
    COLOR_COUNT* this = (COLOR_COUNT*) a;
    COLOR_COUNT* other = (COLOR_COUNT*) b;

    float this_hue = get_hue(this->red_val, this->green_val, this->blue_val);
    float other_hue = get_hue(other->red_val, other->green_val, other->blue_val);

    return (this_hue < other_hue) - (this_hue > other_hue);
}

int as_hex(COLOR_COUNT* cc)
{
    return (cc->red_val << 16) | (cc->green_val << 8) | cc->blue_val;
}

int compare_hex(const void *a, const void *b)
{
    COLOR_COUNT* this = (COLOR_COUNT*) a;
    COLOR_COUNT* other = (COLOR_COUNT*) b;

    int this_hex = as_hex(this);
    int other_hex = as_hex(other);
    
    return (this_hex > other_hex) - (this_hex < other_hex);
}

int within_bound(int anchor, int value)
{
    return (value <= (anchor + BOUND))
        && (value >= (anchor - BOUND));
}

int locate_approx(PIXEL p)
{
    for (int i = 0; i < 255; i++) {
        COLOR_COUNT cc = cc_arr[i];

        int curr_r = cc.red_val;
        int curr_g = cc.green_val;
        int curr_b = cc.blue_val;       

        if (within_bound(cc.red_val, p.red_val) &&
            within_bound(cc.green_val, p.green_val) &&
            within_bound(cc.blue_val, p.blue_val))
        {
            return i;
        }            
    }
    return -1;
}

int locate(PIXEL p)
{
    for (int i = 0; i < 255; i++) {
        COLOR_COUNT cc = cc_arr[i];

        if ((p.red_val == cc.red_val)
            && (p.green_val == cc.green_val)
            && p.blue_val == cc.blue_val)
        {
            return i;
        }
    }
    return -1;
}

void ins_or_inc(PIXEL p, int c);

void compress()
{
    // most inefficient approach, create a new array
    // and re-insert all the items with the new increased bound  
    num = 0;
    COLOR_COUNT* cc_temp = cc_arr;
    cc_arr = malloc(sizeof(COLOR_COUNT) * COLOR_LIMIT);
  
    for (int i = 0; i < COLOR_LIMIT; i++) {
        

    }
        
    
}

void ins_or_inc(PIXEL p, int c)
{
//    int pos = locate_approx(p);
    if (num > COLOR_LIMIT) {
//        compress();
    }
    
    int pos = locate_approx(p);
    
    if (pos == -1) {
        cc_arr[num++] = (COLOR_COUNT) {
            .blue_val = p.blue_val,
            .red_val = p.red_val,
            .green_val = p.green_val,
            .count = c,
        };        
    } else {
        cc_arr[pos].count += c;
    }
    
    qsort(cc_arr, num, sizeof(COLOR_COUNT), compare_hue);
}

int main()
{
    const char* filename = "noise.tga";
//    const char* filename = "redsort.tga";
    FILE *file = fopen(filename, "r");

    TARGA_HEADER tga_header;
    fread(&tga_header, sizeof(char), sizeof(TARGA_HEADER), file);
    
    printf("HEADER-INFO:\nImage Type:\t%d\nWidth:\t\t%dpx\nHeight:\t\t%dpx\n",
           tga_header.img_type,
           tga_header.width,
           tga_header.height);

    cc_arr = malloc(sizeof(COLOR_COUNT) * COLOR_LIMIT);
    
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
            ins_or_inc(pixel, pixc);
        } else {
            pixc = header+1;
            PIXEL pixel;
            for (int pixel_count = header+1; pixel_count > 0; pixel_count--) {
                fread(&pixel, sizeof(uint8_t), sizeof(PIXEL), file);                
                ins_or_inc(pixel, 1);
            }
        }
        curr_pix += pixc;
    }
    fclose(file);

//    qsort(cc_arr, num, sizeof(COLOR_COUNT), compare_hue);

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

    for (int i = 0; i < num; i++) {
        COLOR_COUNT cc = cc_arr[i];

        init_color(PIXEL_COLOR,
                   cc.red_val * COLORSCALE,
                   cc.green_val * COLORSCALE,
                   cc.blue_val * COLORSCALE);
        
        init_pair(PAIR_NUM, COLOR_BLACK, PIXEL_COLOR);

        wattron(palwin, COLOR_PAIR(PAIR_NUM));
        wprintw(palwin, " [%03d,%03d,%03d] ", cc.red_val, cc.green_val, cc.blue_val);
//        wprintw(palwin, " [%02x,%02x,%02x] ", cc.red_val, cc.green_val, cc.blue_val);
        wattroff(palwin, COLOR_PAIR(PAIR_NUM));
        
        PAIR_NUM++;
        PIXEL_COLOR++;
    }
    
    wrefresh(palwin);
    getchar();
    
    endwin();

    
    return 0;
}
