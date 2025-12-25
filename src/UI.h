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
//void set_image_data();

//enum PALETTE_SIZE get_selected_palette_size();
//enum GENERATION_METHOD get_selected_generation_method();
//void initialize_palette(PALETTE* color_palette);

/*
void display_image(TARGA_HEADER header, PIXEL* pixel_data);//, PALETTE* color_palette);
void initialize_palette(PALETTE* color_palette);
*/
/*
    // bit 1: img path
    // bit 2: color count
    // etc...
    get_status_flags()

    get_image_path()
    set_image_data()

    get_color_count()
    get_pal_gen()
    initialize_palette()


    // maybe an internal flag?
    get_render_mode()
*/



//char* get_image_path();
//void set_pixel_data(PIXEL* pixel_data);
//void set_header_info(TARGA_HEADER header);

#endif
