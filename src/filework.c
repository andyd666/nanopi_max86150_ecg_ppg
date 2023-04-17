/*
 * filename: filework.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>


static char *debug_fname = "debug.txt";
static FILE *debug_file;


void init_debug() {
    debug_file = fopen(debug_fname, "a");
    if (!debug_file) {
        printf("%s: Cannot open debug file\n", __func__);
    }
}

void close_debug() {
    if (debug_file) {
        fclose(debug_file);
    }
}

void d_print(const char *__format, ...) {
    va_list list;
    va_start(list, __format);
    if (debug_file) {
        vfprintf(debug_file, __format, list);
        fflush(debug_file);
    } else {
        printf("%s: debug file is not opened, console output: ", __func__);
        vprintf(__format, list);
    }
    va_end(list);

}
