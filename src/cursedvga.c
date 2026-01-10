#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "UI.h"
#include "palette.h"
#include "tga.h"

#include <ncurses.h>

// TODO: move these functions into a util file
int is_whitespace(char c)
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\12';
}

char* ltrim(char* str)
{
    while (*str && is_whitespace(*str)) str++;
    return str;
}

char* rtrim(char* str)
{
    char* end = str + strlen(str);
    while(*str && is_whitespace(*--end));
    *(end+1) = '\0';
    return str;
}

char* trim(char* str)
{
    return rtrim(ltrim(str));
}

IMAGE* try_read_image(const char* filepath)
{
    if (access(filepath, F_OK) != 0) {
        return NULL;
    }

    FILE* file = fopen(filepath, "r");

    fseek(file, 0, SEEK_END);
    int filesize = ftell(file);
    rewind(file);

    uint8_t* bytestream = malloc(filesize);
    fread(bytestream, sizeof(uint8_t), filesize, file);
    fclose(file);

    TARGA_HEADER* header = parse_header(bytestream);

    uint64_t total_pixels = header->height * header->width;
    PIXEL* pixel_data = parse_tga(bytestream);
    free(bytestream);

    // bundle everything together
    IMAGE* image_data = malloc(sizeof(IMAGE));
    *image_data = (IMAGE) {
        header,
        pixel_data
    };

    return image_data;
}

int main(int argc, char *argv[])
{
    initialize_UI();

    IMAGE* image_data = NULL;
    uint8_t return_bitflags;

    // TODO: clean up where mallocs occur, place them in a consistent, predictable place
    while ((return_bitflags = navigate_UI())) {
        if (return_bitflags & NO_UPDATE) {
            continue;
        }
        // how to indicate a failed read?
        // want to assign image_data to null;
        if (return_bitflags & FILEPATH_UPDATE) {
            image_data = try_read_image(trim(get_filepath()));

            if (image_data != NULL) {
                PALETTE color_palette;

                generate_palette(&color_palette, image_data, COMPRESSED_216, COLOR_CUBE);

                // BUG: knife.tga causes "malloc: corrupted top size" when called with FIRST_COLORS_FOUND gen method
                initialize_palette(&color_palette);

                display_image(*image_data->header, image_data->data);
                refresh();
            }
        }
    }
    end_UI();
}
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
