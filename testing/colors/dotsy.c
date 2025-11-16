#include <stdio.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main() {
    system("");
    printf("Try programiz.pro\n");

    for (int i = 0; i < 256; i++) {
	for (int j = 0; j < 256; j++) {
	    for (int k = 0; k < 256; k++) {
		printf("\x1b[38;2;%u;%u;%um Hello World\x1b[0m\n");
	    }
	}
    }

    return 0;
}
