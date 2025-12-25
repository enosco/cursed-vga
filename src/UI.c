#include "UI.h"

#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <form.h>
#include <menu.h>

static const int NCURSES_NEW_PAIR_OFFSET = 2;
static const int NCURSES_NEW_COLOR_OFFSET = 8;

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


/*
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
*/
