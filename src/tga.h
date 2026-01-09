#ifndef TGA_H
#define TGA_H

#include <stdint.h>

/*
 * Header of a TARGA file in order of each field
 */
typedef struct __attribute__((packed)) {
    uint8_t id_len;
    uint8_t color_map_type;
    uint8_t image_type;
    uint16_t color_map_origin;
    uint16_t color_map_len;
    uint8_t color_map_entry_size;
    uint16_t x_orig;
    uint16_t y_orig;
    uint16_t width;
    uint16_t height;
    uint8_t pixel_depth;
    uint8_t img_desc_byte;
} TARGA_HEADER;

/*
 * RGB values of a single pixel in BGR order to match
 * order and bits used by a TARGA file using 24-bit color.
 */
typedef struct __attribute__((packed)) {
    uint8_t blue_val;
    uint8_t green_val;
    uint8_t red_val;
} PIXEL;

typedef struct {
    TARGA_HEADER header;
    PIXEL* data;
} IMAGE;

/*
 * enums for the two types of pixel packet present in a TARGA file
 */
enum PACKET_TYPE {
    RAW,
    RUNLEN
};

// TODO: make extern later
static const char* const TGA_SIGNATURE = "TRUEVISION-XFILE";

// bytestream represents an array of bytes formatted like a TGA file
void parse_tga(PIXEL* pixel_data, const void* bytestream);
TARGA_HEADER parse_header(const void* bytestream);

#endif
