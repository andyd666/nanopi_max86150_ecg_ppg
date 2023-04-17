/**
 * filename: main.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <max86150_defs.h>
#include <filework.h>
#include <peripheral.h>

#define UNUSED(x) ((void)x)


int main(int argc, char **argv) {
    UNUSED(argc);
    UNUSED(argv);

    init_debug();
    d_print("retval from init_gpio() - 0x%2x\n", init_gpio());

    close_debug();
    return 0;
}


/*
static int validate_input(int argc, char **argv) {



    return 0;
}
*/
