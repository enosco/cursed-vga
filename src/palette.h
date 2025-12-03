#ifndef PALETTE_H
#define PALETTE_H

#include "tga.h"

enum PALETTE_SIZE {
    COMPRESSED_248 = 248, // 256 minus 8 to preserve default colors
    COMPRESSED_216 = 216,
    COMPRESSED_128 = 128,
    COMPRESSED_64  = 64,
    COMPRESSED_32  = 32,
    COMPRESSED_16  = 16,
};

enum GENERATION_METHOD {
    COLOR_CUBE,
    FIRST_COLORS_FOUND,
    MOST_COMMON_COLORS,
    K_MEANS_CLUSTERING,
};

typedef struct {
    enum PALETTE_SIZE size;
    PIXEL* data;
} PALETTE;

void generate_palette(PALETTE* palette_data);

#endif
