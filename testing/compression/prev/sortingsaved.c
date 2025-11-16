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

typedef enum {
    RED = 0,
    ORANGE,
    YELLOW,
    GREEN,
    BLUE,
    INDIGO,
    VIOLET
} ROYGBIV;

const PIXEL RED_VAL = {
    .red_val = 255,
    .green_val = 0,
    .blue_val = 0
};

const PIXEL ORANGE_VAL = {
    .red_val = 255,
    .green_val = 128,
    .blue_val = 0
};

const PIXEL YELLOW_VAL = {
    .red_val = 255,
    .green_val = 255,
    .blue_val = 0
};

const PIXEL GREEN_VAL = {
    .red_val = 0,
    .green_val = 255,
    .blue_val = 0
};

const PIXEL BLUE_VAL = {
    .red_val = 0,
    .green_val = 0,
    .blue_val = 255
};
/*
const PIXEL INDIGO_VAL = {
    .red_val = 64,
    .green_val = 0,
    .blue_val = 255
};
*/
const PIXEL VIOLET_VAL = {
    .red_val = 128,
    .green_val = 0,
    .blue_val = 255
};

typedef struct {
    ROYGBIV color;
    float hue;
    int num;
} COLOR_TABLE_HEADER;

COLOR_TABLE_HEADER headers[6] = {
    {RED, RED_VAL, 0},
    {ORANGE, ORANGE_VAL, 0},
    {YELLOW, YELLOW_VAL, 0},
    {GREEN, GREEN_VAL, 0},
    {BLUE, BLUE_VAL, 0},
//    {INDIGO, INDIGO_VAL, 0},
    {VIOLET, VIOLET_VAL, 0},
};

int BOUND = 0;
const int COLOR_LIMIT = 216;

//first dimension is parallel with headers
PIXEL freq[7][COLOR_LIMIT];

int is_color_null(PIXEL p)
{
    return p.red_val == 0
        && p.green_val == 0
        && p.blue_val == 0;
}    

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

int squared(int i)
{
    return i * i;
}

int get_dist_squared(PIXEL anchor, PIXEL curr)
{
    return squared(anchor.red_val - curr.red_val)
        + squared(anchor.green_val - curr.green_val)
        + squared(anchor.blue_val - curr.blue_val);
}
ROYGBIV get_nearest_color(PIXEL p)
{
    ROYGBIV nearest_color = RED;
    int shortest_dist = INT32_MAX;
    for (int i = 0; i < 7; i++) {
        int dist = get_dist_squared(headers[i].color_value, p);
        if (dist < shortest_dist) {
            shortest_dist = dist;
            nearest_color = headers[i].color;
        }
    }
    return nearest_color;
}


int colors_equal(PIXEL a, PIXEL b)
{
    return a.red_val == b.red_val
        && a.green_val == b.green_val
        && a.blue_val == b.blue_val;
}

// returns 2 item array of xy position
void locate(PIXEL p, int* coords)
{
    int row = get_nearest_color(p);
    coords[0] = row;

    COLOR_TABLE_HEADER header = headers[row];
    
    for (int i = 0; i < header.num; i++) {
        if (colors_equal(p, header.color_value)) {
            coords[1] = i;
        }
    }
}

void insert_if_absent(PIXEL p)
{
    int coords[2] = {-1, -1};
    locate(p, coords);
    
    if (coords[1] == -1) {
//        printf("coords: %d, %d\n", coords[0], coords[1]);
        
        freq[coords[0]][headers[coords[0]].num++] = p;
    }    
}

int main()
{
    const char* filename = "_small.tga";
//    const char* filename = "redsort.tga";
    FILE *file = fopen(filename, "r");

    TARGA_HEADER tga_header;
    fread(&tga_header, sizeof(char), sizeof(TARGA_HEADER), file);
    
    printf("HEADER-INFO:\nImage Type:\t%d\nWidth:\t\t%dpx\nHeight:\t\t%dpx\n",
           tga_header.img_type,
           tga_header.width,
           tga_header.height);
    
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

//    qsort(cc_arr, num, sizeof(COLOR_COUNT), compare_hue);
/*
    PIXEL colors[COLOR_LIMIT];
    int i = 0;
    for (int hidx = 0; hidx < 7; hidx++) {
        COLOR_TABLE_HEADER header = headers[hidx];
            
        for (int col = 0; col < COLOR_LIMIT; col++) {
            PIXEL p = freq[header.color][col];
            if (!is_color_null(p)) {
                colors[i++] = p;
            }
        }
    }
*/   
    
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
/*
     for (int i = 0; i < COLOR_LIMIT; i++) {
        PIXEL p = colors[i];

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
    */
    
    for (int hidx = 0; hidx < 6; hidx++) {
        COLOR_TABLE_HEADER header = headers[hidx];
            
        for (int col = 0; col < COLOR_LIMIT; col++) {
            PIXEL p = freq[header.color][col];
             
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
        wclear(palwin);
        PAIR_NUM = 2;
        PIXEL_COLOR = 8;
    }
    
    endwin();
    
    
    return 0;
}
