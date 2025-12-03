#include <ncurses.h>

#include "UI.h"

static const int NCURSES_PAIR_OFFSET = 2;
static const int NCURSES_COLOR_OFFSET = 8;

PALETTE* curr_color_palette;

void initialize_UI()
{
    initscr();
    curs_set(0);
    start_color();
}

void end_UI()
{
    endwin();
}

int compare_color_hex(PIXEL rgb1, PIXEL rgb2)
{
    int rgb1hex = rgb1.red_val << 16 | rgb1.green_val << 8 | rgb1.blue_val;
    int rgb2hex = rgb2.red_val << 16 | rgb2.green_val << 8 | rgb2.blue_val;

    return rgb1hex - rgb2hex;
}

int square(int val)
{
    return val*val;
}

int get_dist_squared(PIXEL rgb1, PIXEL rgb2)
{
    return square(rgb2.red_val - rgb1.red_val)
    + square(rgb2.green_val - rgb1.green_val)
    + square(rgb2.blue_val - rgb1.blue_val);

}

int get_closest_color_pair(PIXEL target_color)
{
    // TODO: this can be optimized later, just get it working for now
    int closest_index = 0;
    int shortest_dist = INT32_MAX;

    for (int i = 0; i < curr_color_palette->size; i++) {
        int curr_dist;
        PIXEL curr_color = curr_color_palette->data[i];
        if ((curr_dist = get_dist_squared(target_color, curr_color)) < shortest_dist) {
            closest_index = i;
            shortest_dist = curr_dist;
        }
    }

    refresh();
    return closest_index + NCURSES_PAIR_OFFSET;
}

void initialize_palette(PALETTE* color_palette)
{
    const double color_scale = 1000.0 / 255.0;

    for (int i = 0; i < color_palette->size; i++) {
        PIXEL curr = color_palette->data[i];
        init_color(i + NCURSES_COLOR_OFFSET,
                   curr.red_val * color_scale,
                   curr.green_val * color_scale,
                   curr.blue_val * color_scale);

        init_pair(i + NCURSES_PAIR_OFFSET,
                  i + NCURSES_COLOR_OFFSET,
                  i + NCURSES_COLOR_OFFSET);
    }
    curr_color_palette = color_palette;
}

void display_image(TARGA_HEADER header, PIXEL* pixel_data)//, PALETTE* color_palette)
{
    WINDOW* img_win = newwin(header.height*4, header.width*4, 0, 0);
    WINDOW* log_win = newwin(64, 32, 0, header.width*2+4);

    refresh();
    wrefresh(img_win);

    int total_pixels = header.height * header.width;

    int pixel_pos = 0;
    double COLOR_SCALE = 1000.0 / 255.0;


    for (int row = header.height-1; row >= 0; row--) {
        for (int col = 0; col < header.width*2; col+=2) {

            PIXEL curr = pixel_data[pixel_pos++];

            int color_pair = get_closest_color_pair(curr);//, color_palette);
            mvwchgat(img_win, row, col, 2, 0, color_pair, NULL);
        }
    }

    wrefresh(log_win);
    wrefresh(img_win);

    refresh();
}

