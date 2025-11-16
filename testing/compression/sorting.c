#include <limits.h>
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

typedef struct __attribute__((packed)) {
    unsigned char blue_val;
    unsigned char green_val;
    unsigned char red_val;
} PIXEL;

typedef struct {
    PIXEL pixel;
    int frequency;
} COLOR_FREQ;

typedef enum {
    RED = 0,
    ORANGE,
    YELLOW,
    GREEN,
    BLUE,
    VIOLET
} ROYGBIV;


const PIXEL NULL_PIXEL = {
    .red_val = 0,
    .green_val = 0,
    .blue_val = 0
};

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
    {RED, 0, 0},
    {ORANGE, 30, 0},
    {YELLOW, 60, 0},
    {GREEN, 90, 0},
    {BLUE, 240, 0},
    {VIOLET, 270, 0},
};

int BOUND = 0;
#define COLOR_LIMIT 216

//first dimension is parallel with headers
COLOR_FREQ freq[7][COLOR_LIMIT];

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
    PIXEL* this = (PIXEL*) a;
    PIXEL* other = (PIXEL*) b;

    float this_hue = get_hue(this->red_val, this->green_val, this->blue_val);
    float other_hue = get_hue(other->red_val, other->green_val, other->blue_val);

    return (this_hue < other_hue) - (this_hue > other_hue);
}

int as_hex(PIXEL* cc)
{
    return (cc->red_val << 16) | (cc->green_val << 8) | cc->blue_val;
}

int compare_hex(const void *a, const void *b)
{
    PIXEL* this = (PIXEL*) a;
    PIXEL* other = (PIXEL*) b;

    int this_hex = as_hex(this);
    int other_hex = as_hex(other);

    return (this_hex > other_hex) - (this_hex < other_hex);
}

int squared(int i)
{
    return i * i;
}


int get_distance_squared(PIXEL a, PIXEL b) 
{
    return squared(a.red_val-b.red_val)
    + squared(a.green_val-b.green_val)
    + squared(a.blue_val-b.blue_val);
}


ROYGBIV get_nearest_major_color(PIXEL p)
{
    ROYGBIV nearest_color = RED;
    float smallest_diff = INT32_MAX;
    for (int i = 0; i < 6; i++) {
        int hue = get_hue(p.red_val,p.green_val,p.blue_val);
        int diff = fabs(hue - headers[i].hue);
        if (diff < smallest_diff) {
            smallest_diff = diff;
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
    int row = get_nearest_major_color(p);
    coords[0] = row;

    COLOR_TABLE_HEADER header = headers[row];

    for (int i = 0; i < header.num; i++) {
        if (colors_equal(p, freq[header.color][i].pixel)) {
            coords[1] = i;
        }
    }
}

void delete_and_shift(ROYGBIV table, int index)
{
    int last_i = headers[table].num - 1;
    for (int i = index; i < last_i; i++) {
        freq[table][index] = freq[table][index+1];
    }
    // set last item to "null", 
    freq[table][last_i].pixel = NULL_PIXEL;
    freq[table][last_i].frequency = 0;

    headers[table].num--;
}

void merge_with_nearest(ROYGBIV table_index, int color_index)
{
    COLOR_TABLE_HEADER header = headers[table_index];
    COLOR_FREQ* table = freq[table_index];

    // find nearest color
    COLOR_FREQ target = freq[table_index][color_index];

    int nearest_i = 0;
    int nearest_dist_sq = INT_MAX;
    for (int i = 0; i < header.num; i++) {
        if (i == color_index) {
            continue;
        }

        int curr_dist_sq = get_distance_squared(target.pixel, freq[table_index][i].pixel);

        if (curr_dist_sq < nearest_dist_sq) {
            nearest_dist_sq = curr_dist_sq;
            nearest_i = i;
        }
    }

    // add frequency to target before merge
    target.frequency += freq[table_index][nearest_i].frequency;

    delete_and_shift(table_index, nearest_i);
}

void compress(int table_index)
{
    // get most common color in largest array
    // merge it with its nearest color

    int mfreq_index = 0;
    for (int i = 0; i < headers[table_index].num; i++) {
        if (freq[table_index][mfreq_index].frequency < freq[table_index][i].frequency) {
            mfreq_index = i;
        }
    }

    PIXEL mfreq_pixel = freq[table_index][mfreq_index].pixel;

    /*
    printf("MCC: %d, %d, %d in table %d occurs %d times\n\n",
           p.red_val,
           p.green_val,
           p.blue_val,
           table_index,
           freq[table_index][mfreq_index].frequency
           );
    */
    merge_with_nearest(table_index, mfreq_index);
}

int is_limit_hit()
{
    int total = 0;
    int largest_table = RED;
    for (int i = 0; i < 6; i++) {
        if (headers[largest_table].num < headers[i].num) {
            largest_table = headers[i].color;
        }

        total += headers[i].num;
    }
    if (total > COLOR_LIMIT) {
        return largest_table;
    } else {
        return -1;
    }
}

void insert_if_absent(COLOR_FREQ cf)
{
    int coords[2] = {-1, -1};
    locate(cf.pixel, coords);
    int row = coords[0];
    int col = coords[1];

    // Check if this color is already in the color table
    if (col == -1) {
        freq[row][headers[coords[0]].num++] = cf;
    } else {
        freq[row][col].frequency += cf.frequency;
    }

    int largest_table;
    if ((largest_table = is_limit_hit()) != -1) {
        compress(largest_table);
    }
}

int main(int argc, char** args)
{
    // read in file and header
    const char* filename = args[1];
    FILE *file = fopen(filename, "r");

    TARGA_HEADER tga_header;
    fread(&tga_header, sizeof(char), sizeof(TARGA_HEADER), file);

    printf("HEADER-INFO:\nImage Type:\t%d\nWidth:\t\t%dpx\nHeight:\t\t%dpx\n",
           tga_header.img_type,
           tga_header.width,
           tga_header.height);

    // read colors from the file and insert into the color tables
    int limit = tga_header.width * tga_header.height;
    int curr_pix = 0;
    while (curr_pix < limit) {
        uint8_t header;
        fread(&header, sizeof(uint8_t), sizeof(uint8_t), file);

        int type = header >> 7;

        int pixc = 0;

        if (type) { // runlength
            pixc = (header ^ 0x80) + 1;

            PIXEL pixel;
            fread(&pixel, sizeof(uint8_t), sizeof(PIXEL), file);            
            insert_if_absent(
                (COLOR_FREQ) {
                    .pixel = pixel,
                    .frequency = pixc
                });

        } else { // raw
            pixc = header+1;
            PIXEL pixel;
            for (int pixel_count = header+1; pixel_count > 0; pixel_count--) {
                fread(&pixel, sizeof(uint8_t), sizeof(PIXEL), file);

                insert_if_absent(
                    (COLOR_FREQ) {
                        .pixel = pixel,
                        .frequency = 1
                    });
            }
        }
        curr_pix += pixc;
    }
    fclose(file);


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

    for (int hidx = 0; hidx < 6; hidx++) {
        mvprintw(0, 0, "TABLE: %d", hidx);
        COLOR_TABLE_HEADER header = headers[hidx];

        for (int col = 0; col < COLOR_LIMIT; col++) {

            mvprintw(col, 91, "FREQ: %d", freq[header.color][col].frequency);
            PIXEL p = freq[header.color][col].pixel;

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

        refresh();
        wrefresh(palwin);

        getchar();
        clear();
        wclear(palwin);

        PAIR_NUM = 2;
        PIXEL_COLOR = 8;
    }

    endwin();


    return 0;
}
