cursedvga is a command line tool that uses NCurses to decode and display TGA images. In adherence to NCurses' limitations (and because I think its neat) colors are mapped to their nearest match within an 8-bit color palette. Future plans include adding a more robust TUI, several ways of generating a color-palette, and LED simulation. 

Currently, cursedvga only supports a subset of TGA image types, that being run-length encoded RGB images (without a color map).
