#define _XOPEN_SOURCE_EXTENDED

#include <ncurses.h>
#include <wchar.h>
#include <wctype.h>
#include <locale.h>

#include "UI.h"

static const int NCURSES_PAIR_OFFSET = 2;
static const int NCURSES_COLOR_OFFSET = 8;

PALETTE* curr_color_palette;

void initialize_UI()
{
    setlocale(LC_ALL, "");

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

int dist_squared(PIXEL rgb1, PIXEL rgb2)
{
    return square(rgb2.red_val - rgb1.red_val)
    + square(rgb2.green_val - rgb1.green_val)
    + square(rgb2.blue_val - rgb1.blue_val);

}

int find_nearest_color(PIXEL target_color)
{
    // TODO: this can be optimized later, just get it working for now
    int closest_index = 0;
    int shortest_dist = INT32_MAX;

    for (int i = 0; i < curr_color_palette->size; i++) {
        int curr_dist;
        PIXEL curr_color = curr_color_palette->data[i];
        if ((curr_dist = dist_squared(target_color, curr_color)) < shortest_dist) {
            closest_index = i;
            shortest_dist = curr_dist;
        }
    }
    return closest_index + NCURSES_COLOR_OFFSET;
}

// Returns the color pair number matching the given fg & bg combination.
// Pair number will be negative if the inverse of the color exists.
// Allocates a new color pair if neither this combination nor the inverse already
// exists.
int find_nearest_pair(uint8_t fg, uint8_t bg)
{
    int pair_num;

    // check if inverse pair exists before attempting to create a new pair
    if ((pair_num = find_pair(bg, fg)) != -1) {
        return -pair_num;
    } else {
        return alloc_pair(fg, bg);
    }
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
    }

    curr_color_palette = color_palette;
}

void display_color_pair(WINDOW* win, int y, int x, uint16_t fg, uint16_t bg)
{
    static const wchar_t* half_block = L"\u2580";

    int pair = find_nearest_pair(fg, bg);

    if (pair < 0) {
        wattr_set(win, A_REVERSE, -pair ,NULL);
    } else {
        wattr_set(win, 0, pair, NULL);
    }

    mvwaddwstr(win, y, x, half_block);
}

void display_image(TARGA_HEADER header, PIXEL* pixel_data)//, PALETTE* color_palette)
{
    WINDOW* img_win = newwin(header.height*4, header.width*4, 0, 0);

    refresh();
    wrefresh(img_win);


    int pixel_pos = 0;
    for (int row = 0; row < (header.height / 2); row++) {
        for (int col = 0; col < header.width; col++) {
            int top_color = find_nearest_color(pixel_data[pixel_pos]);
            int bottom_color = find_nearest_color(pixel_data[pixel_pos + header.width]);

            display_color_pair(img_win, row, col, top_color, bottom_color);

            pixel_pos++;
        }
        pixel_pos += header.width;
    }

    // If image height is odd, the last row still needs to be printed
    if (header.height % 2 != 0) {

        refresh();
        int last_index = header.height * header.width;
        //int row_pos = last_index - header.width;

        for (int col = 0; col < header.width; col++) {
            int top_color = find_nearest_color(pixel_data[pixel_pos++]);

            display_color_pair(img_win,
                               header.height / 2,
                               col,
                               top_color,
                               COLOR_BLACK);
        }
    }

    wrefresh(img_win);
}

