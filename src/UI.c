#include <ncurses.h>

#include "UI.h"

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

void display_image(TARGA_HEADER header, PIXEL* pixel_data)
{
    initialize_UI();

    WINDOW* img_win = newwin(header.height*4, header.width*4, 0, 0);
    WINDOW* log_win = newwin(64, 32, 0, header.width*2+4);

    refresh();
    wrefresh(img_win);

    int total_pixels = header.height * header.width;

    int PAIR = 2;
    int COLOR = 8;

    int pixel_pos = 0;
    double COLOR_SCALE = 1000.0 / 255.0;
    for (int row = header.height-1; row >= 0; row--) {
        for (int col = 0; col < header.width*2; col+=2) {

            PIXEL curr = pixel_data[pixel_pos++];

            init_color(COLOR,
                       curr.red_val * COLOR_SCALE,
                       curr.green_val * COLOR_SCALE,
                       curr.blue_val * COLOR_SCALE);

            init_pair(PAIR, COLOR, COLOR);

            mvwchgat(img_win, row, col, 2, 0, PAIR, NULL);

            PAIR++;
            COLOR++;

        }
    }

    wrefresh(log_win);
    wrefresh(img_win);


    getchar();

    end_UI();
}

