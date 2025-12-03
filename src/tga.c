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

void parse_tga(PIXEL* pixel_data, const void* bytestream)
{
    uint8_t* stream_pos = (uint8_t*) bytestream + sizeof(TARGA_HEADER);
    TARGA_HEADER header = parse_header(bytestream);
    uint64_t total_pixels = header.height * header.width;

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

}

TARGA_HEADER parse_header(const void* bytestream)
{
    TARGA_HEADER res;
    memcpy(&res, bytestream, sizeof(TARGA_HEADER));
    return res;
}


