#ifndef UI_H
#define UI_H

#include "tga.h"
#include "palette.h"

void initialize_UI();
void end_UI();

void display_image(TARGA_HEADER header, PIXEL* pixel_data);
void initialize_palette(PALETTE* color_palette);

//char* get_filepath();
//void set_pixel_data(PIXEL* pixel_data);
//void set_header_info(TARGA_HEADER header);

#endif
