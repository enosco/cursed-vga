#include <stdlib.h>
#include <stdio.h>
#include "palette.h"

void generate_color_cube(PALETTE* palette)
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

void generate_palette(PALETTE* palette) //, enum PALETTE_SIZE size, enum GENERATION_METHOD gen_method)
{
    // palette should be formatted in a way easily searchable by rgb,
    // maybe sorted by rgb as hex?
    palette->size = COMPRESSED_216;
    generate_color_cube(palette);

    // TODO: implement other color gen methods
    // for now, just use 216 color cube
}
