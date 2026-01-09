#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "UI.h"
#include "palette.h"
#include "tga.h"

int main(int argc, char *argv[])
{
    initialize_UI();
    
    FILE* file;

    uint8_t return_bitflags;
    while ((return_bitflags = navigate_UI())) {
        if (return_bitflags & NO_UPDATE) {
            continue;
        }

        if (return_bitflags & FILEPATH_UPDATE) {
            //file = fopen(get_filepath(), "r");
            //validate_file(file);
            printf("%s", get_filepath());
            if (access(get_filepath(), F_OK) == 0) {
                printf("file exists");
            }
        }
    }

    end_UI();
/*
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

    // bundle everything together
    IMAGE image = (IMAGE) {
        header,
        pixel_data
    };
    
    PALETTE color_palette;

    generate_palette(&color_palette, &image, COMPRESSED_216, FIRST_COLORS_FOUND);
    initialize_UI();


    initialize_palette(&color_palette);
    display_image(header, pixel_data);//, color_palette);

    getchar();

    end_UI();
    */

    return 0;
}
