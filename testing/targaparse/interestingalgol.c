#include <locale.h>
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
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

typedef struct {
    unsigned char blue_val;
    unsigned char green_val;
    unsigned char red_val;
    int count;
} COLOR_COUNT;

typedef struct {
    unsigned char blue_val;
    unsigned char green_val;
    unsigned char red_val;
} PIXEL;


int num = 0;
//COLOR_COUNT* cc_arr;
const int COLOR_LIMIT = 248;
int BOUND = 12;
COLOR_COUNT* cc_arr;


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

void ins_or_inc(PIXEL p, int c)
{
    int pos = locate_approx(p);
//    int pos = locate(p);
    
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
}
int main()
{
    const char* filename = "knife.tga";
    FILE *file = fopen(filename, "r");

    TARGA_HEADER tga_header;
    fread(&tga_header, sizeof(char), sizeof(TARGA_HEADER), file);

    
    printf("HEADER-INFO:\nImage Type:\t%d\nWidth:\t\t%dpx\nHeight:\t\t%dpx\n",
           tga_header.img_type,
           tga_header.width,
           tga_header.height);

    cc_arr = malloc(sizeof(COLOR_COUNT) *
                    (tga_header.width * tga_header.height));    
    
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

            /*
            printf("T[%d]\tC[%d]\tRGB[%d,%d,%d]\n",
                   type,
                   pixc,
                   pixel.red_val,
                   pixel.green_val,
                   pixel.blue_val);
            */
            
            ins_or_inc(pixel, pixc);
        } else {
            pixc = header+1;
            PIXEL pixel;
            for (int pixel_count = header+1; pixel_count > 0; pixel_count--) {
                fread(&pixel, sizeof(uint8_t), sizeof(PIXEL), file);

                /*
                printf("T[%d]\tC[%d]\tRGB[%d,%d,%d]\n",
                       type,
                       1,
                       pixel.red_val,
                       pixel.green_val,
                       pixel.blue_val);
                */
                ins_or_inc(pixel, 1);
            }
        }
        curr_pix += pixc;
    }
    
    for (int i = 0; i < num; i++) {
        COLOR_COUNT cc = cc_arr[i];

        printf("RGB: (%d, %d, %d) -> %d\n", cc.red_val,
               cc.green_val, cc.blue_val, cc.count);
    }
    printf("size: %d\n", num);
    
    fclose(file);
/*
    initscr();
    start_color();
    curs_set(0);
    WINDOW* palwin = newwin(80,32,0,0);
    refresh();
    
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

        waddch(palwin, ' ' | COLOR_PAIR(PAIR_NUM));

        PAIR_NUM++;
        PIXEL_COLOR++;
    }
    
    wrefresh(palwin);
    getchar();
    
    endwin();
*/
    
    return 0;
}
