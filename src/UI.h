#ifndef UI_H
#define UI_H

#include "tga.h"
#include "palette.h"

enum STATUS_FLAG {
    EXIT_REQUESTED = 0,
    NO_UPDATE = 1,
    FILEPATH_UPDATE = 2
};

void initialize_UI();
void end_UI();

// returns bitflag byte aligning with STATUS_FLAGS enum
uint8_t navigate_UI();
char* get_filepath();

void initialize_palette(PALETTE* color_palette);
void display_image(TARGA_HEADER header, PIXEL* pixel_data);

#endif
