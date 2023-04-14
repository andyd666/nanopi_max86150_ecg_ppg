/**
 * filename: main.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>


#define UNUSED(x) ((void)x)

int validate_input(int argc, char **argv) {
    int i;
    printf("Hello world!\nargc = %d, argv = ", argc);

    for (i = 0; i < argc; i++) {
        printf("%s ", argv[i]);
    }
    printf("\n");

    return 0;
}

int main(int argc, char **argv) {

    return validate_input(argc, argv);
}
