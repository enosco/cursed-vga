#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "UI.h"
#include "palette.h"
#include "tga.h"

int main(int argc, char *argv[])
{

    if (argc < 2) {
        printf("ERR: No Filename Specified\n");
        return 1;
    }

    const char *filename = argv[1];

    FILE *file = fopen(filename, "r");

    fseek(file, 0, SEEK_END);
    int filesize = ftell(file);
    rewind(file);

    uint8_t* bytestream = malloc(filesize);
    fread(bytestream, sizeof(uint8_t), filesize, file);
    fclose(file);

    TARGA_HEADER header = parse_header(bytestream);

    printf("Path:\t\t%s\nWidth:\t\t%dpx\nHeight:\t\t%dpx\nImage Type:\t%d\nPixel Depth:\t%d-bit\n",
           filename, header.width,
           header.height,
           header.image_type,
           header.pixel_depth);

    printf("\nPress enter to display...");
    getchar();


    uint64_t total_pixels = header.height * header.width;
    PIXEL* pixel_data = malloc(total_pixels * sizeof(PIXEL));
    parse_tga(pixel_data, bytestream);
    free(bytestream);

    PALETTE color_palette;
    color_palette.data = malloc(COMPRESSED_216 * sizeof(PIXEL));
    generate_palette(&color_palette);
    initialize_UI();

    initialize_palette(&color_palette);
    display_image(header, pixel_data);//, color_palette);

    getchar();

    end_UI();
    return 0;
}
