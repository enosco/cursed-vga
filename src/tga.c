#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "tga.h"

// assumes stream_pos is positioned at the start of a pixel packet, then updates
// accordingly
void parse_run_length_packet(PIXEL* pixmap, uint64_t insert_index, uint8_t packet_rep_count, uint8_t* stream_pos)
{
    for (; packet_rep_count > 0; packet_rep_count--) {
        memcpy(pixmap + insert_index++, stream_pos, sizeof(PIXEL));
    }
}

void parse_raw_packet(PIXEL* pixmap, uint64_t insert_index, uint8_t packet_pixel_count, uint8_t* stream_pos)
{
    for (; packet_pixel_count > 0; packet_pixel_count--) {
        memcpy(pixmap + insert_index++, stream_pos, sizeof(PIXEL));
        stream_pos += 3;
    }
}

PIXEL* parse_tga(const void* bytestream)
{
    uint8_t* stream_pos = (uint8_t*) bytestream + sizeof(TARGA_HEADER);
    TARGA_HEADER* header = parse_header(bytestream);
    int remaining_pixels = header->height * header->width;

    PIXEL* pixel_data = malloc(sizeof(PIXEL) * remaining_pixels);

    /*
    uint64_t total_pixels = header->height * header->width;
    uint64_t insert_index = 0;
    while (insert_index < total_pixels) {
        uint8_t packet_header = *stream_pos++;
        uint8_t header_type = packet_header >> 7;

        uint8_t pixel_count;
        if (header_type == RUNLEN) { // run-length packet
            pixel_count = (packet_header & 0x7f) + 1;

            parse_run_length_packet(pixel_data, insert_index, pixel_count, stream_pos);
            stream_pos += sizeof(PIXEL) / sizeof(uint8_t);
        } else { // raw packet
            pixel_count = packet_header + 1;
            parse_raw_packet(pixel_data, insert_index, pixel_count, stream_pos);
            stream_pos += sizeof(PIXEL) / sizeof(uint8_t) * pixel_count;
        }

        insert_index += pixel_count;
    }
    */

    // Insert backwards row-by-row so the result array
    // starts with the top left pixel at index 0
    int insert_start_index = remaining_pixels - header->width;
    while (insert_start_index >= 0) {
        int row_pos = insert_start_index;
        int row_limit = row_pos + header->width;

        while (row_pos < row_limit) {
            uint8_t packet_header = *stream_pos++;
            uint8_t header_type = packet_header >> 7;

            uint8_t pixel_count;

            if (header_type == RUNLEN) { // run-length packet
                pixel_count = (packet_header & 0x7f) + 1;
                parse_run_length_packet(pixel_data, row_pos, pixel_count, stream_pos);
                stream_pos += sizeof(PIXEL) / sizeof(uint8_t);
            } else { // raw packet
                pixel_count = packet_header + 1;
                parse_raw_packet(pixel_data, row_pos, pixel_count, stream_pos);
                stream_pos += sizeof(PIXEL) / sizeof(uint8_t) * pixel_count;
            }

            row_pos += pixel_count;
        }
        insert_start_index -= header->width;
    }

    return pixel_data;
}

TARGA_HEADER* parse_header(const void* bytestream)
{
    TARGA_HEADER* header = malloc(sizeof(TARGA_HEADER));
    memcpy(header, bytestream, sizeof(TARGA_HEADER));
    return header;
}


