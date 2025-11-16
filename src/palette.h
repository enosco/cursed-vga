#ifndef PALETTE_H
#define PALETTE_H

#include "tga.h"

enum PALETTE_TYPE {
    TERM_DEFAULT,
    COMPRESSED_248,
    COMPRESSED_216,
    COMPRESSED_128,
    COMPRESSED_64,
    COMPRESSED_32,
    COMPRESSED_16,
};

enum GENERATION_TYPE {
    FIRST_COLORS_FOUND,
    MOST_COMMON_COLOR,
};

PIXEL* generate_palette(enum PALETTE_TYPE palette_type,
                        enum GENERATION_TYPE gen_type,
                        PIXEL* pixel_data);

#endif
