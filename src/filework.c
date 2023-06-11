/*
 * filename: filework.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <filework.h>

static int binary_capture;
static FILE *debug_file;

int open_capture_file(char *name) {
    if (name[0]) {
        binary_capture = open(name, O_CREAT|O_EXCL|O_RDWR, S_IRWXU);
    } else {
        binary_capture = open(DEFAULT_BINARY_NAME, O_CREAT|O_EXCL|O_RDWR, S_IRWXU);
    }
    return binary_capture;
}

int close_capture_file() {
    return (binary_capture != -1) ? close(binary_capture) : -1;
}

void init_debug() {
    debug_file = fopen(DEBUG_FNAME, "a");
    if (!debug_file) {
        printf("%s: Cannot open debug file\n", __func__);
    }
}

void close_debug() {
    if (debug_file) {
        d_print("############################################\n");
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
