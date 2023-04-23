/**
 * filename: main.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <max86150_defs.h>
#include <filework.h>
#include <peripheral.h>

#define UNUSED(x) ((void)x)

static int validate_input(int argc, char **argv, uint16_t *samp_freq, uint16_t *sig);
static void print_usage(char **argv);


int main(int argc, char **argv) {
    uint16_t sig = 0, samp_freq = 0;
    int retval = 0;

    init_debug();

    if (validate_input(argc, argv, &samp_freq, &sig)) {
        /* d_print("%s: cannot validate input\n", __func__); */ /* May be silently closed */
        retval = -1;
        goto cant_start;
    }

    if (init_gpio() == 0) {
        d_print("%s: init_gpio() successful\n", __func__);
    } else {
        d_print("%s: init_gpio() NOT successful\n", __func__);
        retval = -1;
        goto cant_start;
    }

    if (init_max86150(samp_freq, sig)) {
        retval = -1;
        goto cant_start;
    }

cant_start:

    close_debug();
    return retval;
}


static int validate_input(int argc, char **argv, uint16_t *samp_freq, uint16_t *sig) {
    int i;
    if (argc == 1) {
        print_usage(argv);
        return -1;
    }

    for (i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            if (argv[i][1] == 'h') {
                print_usage(argv);
                return -1;
            }
            if (argv[i][1] == 'f') {
                i++;
                *samp_freq = atoi(argv[i]);
                if (!(*samp_freq)) {
                    printf("%s: sample frequency is invalid - %s\n", __func__, argv[i]);
                    return -1;
                }
                continue;
            }
            if (0 == strcmp(argv[i], "--ppg1")) {
                *sig |= ppg1;
                continue;
            }
            if (0 == strcmp(argv[i], "--ppg2")) {
                *sig |= ppg2;
                continue;
            }
            if (0 == strcmp(argv[i], "--ppg")) {
                *sig |= ppg1 | ppg2;
                continue;
            }
            if (0 == strcmp(argv[i], "--pilot1")) {
                *sig |= pilot1;
                continue;
            }
            if (0 == strcmp(argv[i], "--pilot2")) {
                *sig |= pilot2;
                continue;
            }
            if (0 == strcmp(argv[i], "--pilot")) {
                *sig |= pilot1 | pilot2;
                continue;
            }
            if (0 == strcmp(argv[i], "--ecg")) {
                *sig |= ecg;
                continue;
            }
            printf("%s, %d: unknown parameter - \"%s\"\n", __func__, __LINE__, argv[i]);
            print_usage(argv);
            return -1;
        } else {
            printf("%s, %d: unknown parameter - \"%s\"\n", __func__, __LINE__, argv[i]);
            print_usage(argv);
            return -1;
        }
    }
    return 0;
}

static void print_usage(char **argv) {
    printf("%s usage:\n", argv[0]);
    printf("\t-h\t\t-\tshow usage\n");
    printf("\t-f\t\t-\tset sampling frequency\n");
    printf("\t--ppg1\t\t-\ttoggle on PPG1\n");
    printf("\t--ppg2\t\t-\ttoggle on PPG2\n");
    printf("\t--ppg\t\t-\ttoggle on both PPG signals\n");
    printf("\t--pilot1\t-\ttoggle on PILOT1\n");
    printf("\t--pilot2\t-\ttoggle on PILOT2\n");
    printf("\t--pilot\t\t-\ttoggle on both PILOT signals\n");
    printf("\t--ecg\t\t-\ttoggle on ECG signal\n");
    printf("\tNote: \"-f200\" is invalid value. Please, separate flags and values\n");
}
