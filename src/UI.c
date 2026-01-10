#define _XOPEN_SOURCE_EXTENDED

#include <stdlib.h>
#include <string.h>

#include <ncurses.h>
#include <form.h>
#include <menu.h>

#include <wchar.h>
#include <wctype.h>
#include <locale.h>

#include "UI.h"

static const int NCURSES_PAIR_OFFSET = 4;
static const int NCURSES_COLOR_OFFSET = 8;

static const int ACTIVE_UI_ELEM_COLOR_PAIR = 2;
static const int INVALID_UI_ELEM_COLOR_PAIR = 3;

typedef struct {
    int size;
    char* choices[32];
} CHOICES;

typedef struct {
    FORM* form;
    WINDOW* win;
    char* win_title;
} FORM_BUNDLE;

typedef struct {
    MENU* menu;
    ITEM* selected_item;
    WINDOW* win;
    char* win_title;
} MENU_BUNDLE;

FORM_BUNDLE* filepath_bundle;

PALETTE* curr_color_palette;


void attach_title_to_win(WINDOW* win, char* text)
{
    int y,x;
    getmaxyx(win, y, x);

    int midpoint = (x >> 1) - (strlen(text) >> 1);
    wattron(win, A_BOLD);
    mvwprintw(win, 0, midpoint, "%s", text);
    wattroff(win, A_BOLD);
}

void draw_frame_with_title(WINDOW* win, char* text)
{
    box(win, 0, 0);
    attach_title_to_win(win, text);
}

void initialize_UI()
{
    // form for file path
    // menu for palette size:
    // menu for palette generation
    setlocale(LC_ALL, "");

    initscr();
    noecho();
    curs_set(0);
    start_color();
    keypad(stdscr, TRUE);

    refresh();

    /**** Initialize Filepath Form ****/

    filepath_bundle = malloc(sizeof(FORM_BUNDLE));

    const int form_win_h = 3;
    const int form_win_w = 51;

    filepath_bundle->win = newwin(form_win_h, form_win_w, 0, 0);
    filepath_bundle->win_title = " Path to image ";
    keypad(filepath_bundle->win, 1);

    // decorate window
    draw_frame_with_title(filepath_bundle->win, filepath_bundle->win_title);
    char* form_label = "filepath_bundle:";
    mvwprintw(filepath_bundle->win, 1, 1, "%s", form_label);

    FIELD* fields[2];

    const int field_width = form_win_w - strlen(form_label) - 2;
    fields[0] = new_field(1, field_width, 0, 0, 0, 0);
    set_field_back(fields[0], A_STANDOUT);

    fields[1] = NULL;

    filepath_bundle->form = new_form(fields);

    set_form_win(filepath_bundle->form, filepath_bundle->win);
    set_form_sub(filepath_bundle->form, derwin(filepath_bundle->win, 1, field_width, 1,strlen(form_label)+1));
    post_form(filepath_bundle->form);

    wrefresh(filepath_bundle->win);
}

void end_UI()
{
    endwin();
}

char* get_filepath()
{
    form_driver(filepath_bundle->form, REQ_VALIDATION);
    return field_buffer(current_field(filepath_bundle->form), 0);
}

enum STATUS_FLAG accept_form_input(FORM_BUNDLE* form_bundle)
{
    curs_set(1);

    enum STATUS_FLAG status = NO_UPDATE;
    int c = '\0';
    while((c = wgetch(form_bundle->win)) != 10) { // 10: Enter key pressed
        switch(c) {
            case KEY_F(1): // TODO: there should be a better way of signaling an exit
                return EXIT_REQUESTED;
            case KEY_BACKSPACE: // Backspace
                form_driver(form_bundle->form, REQ_LEFT_CHAR);
                form_driver(form_bundle->form, REQ_DEL_CHAR);
                break;
            default:
                form_driver(form_bundle->form, c);
                break;
        }
        status = FILEPATH_UPDATE;
    }
    curs_set(0);

    return status;
}

uint8_t navigate_UI()
{
    uint8_t bitflags = 0;

    // use f1 as exit for now, if pressed send "1"
    bitflags |= accept_form_input(filepath_bundle);

    return bitflags;
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
    WINDOW* img_win = newwin(header.height*4, header.width*4, 0, 52);

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
