#include <stdlib.h>
#include <stdio.h>
#include "palette.h"

void gp_color_cube(PALETTE* palette)
{
    PIXEL* data = palette->data;

    int curr_idx = 0;

    int red_val = 0;
    int red_incr = 0x5f;

    do {
        int green_val = 0;
        int green_incr = 0x5f;
        do {
            int blue_val = 0;
            int blue_incr = 0x5f;

            do {
                data[curr_idx++] = (PIXEL) {
                    .blue_val = blue_val,
                    .green_val = green_val,
                    .red_val = red_val
                };

                blue_val += blue_incr;
                blue_incr = 0x28;

            } while (blue_val < 256);

            green_val += green_incr;
            green_incr = 0x28;
        } while (green_val < 256);

        red_val += red_incr;
        red_incr = 0x28;
    } while (red_val < 256);
}

// TODO: move these utility functions to their own file
// also, modifying the amount bit shifted could be a
// TUI element in the future
int pixels_equal(PIXEL a, PIXEL b)
{
    return a.red_val >> 4 == b.red_val >> 4
    && a.green_val >> 4 == b.green_val >> 4
    && a.blue_val >> 4 == b.blue_val >> 4;
}

// returns -1 if not inserted
int put_if_absent(PALETTE* palette, PIXEL new_pixel, int num_elems)
{
    for (int index = 0; index < num_elems; index++) {
        if (pixels_equal(new_pixel, palette->data[index])) {
            return -1;
        }
    }
    palette->data[num_elems+1] = new_pixel;
    return 0;
}

void gp_first_colors(PALETTE* palette, const IMAGE* image)
{
    int total_pixels = image->header->height * image->header->width;
    int num_elems = 0;
    for (int img_index = 0; img_index < total_pixels && num_elems < palette->size; img_index++) {
        PIXEL p = image->data[img_index];
        if (put_if_absent(palette, p, num_elems) != -1) {
            num_elems++;
        }
    }
}

void generate_palette(PALETTE* palette, const IMAGE* image, enum PALETTE_SIZE size, enum GENERATION_METHOD gen_method)
{
    palette->size = size;
    palette->data = malloc(size * sizeof(PIXEL));

    switch (gen_method) {
        case COLOR_CUBE:
            gp_color_cube(palette);
            break;
        case FIRST_COLORS_FOUND:
            gp_first_colors(palette, image);
            break;
        default:
            break;
    }
}
