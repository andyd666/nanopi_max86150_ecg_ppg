/*
 * filename: filework.h
 */

#ifndef INCLUDE_FILEWORK_H_
#define INCLUDE_FILEWORK_H_

#define DEFAULT_BINARY_NAME "/tmp/ecg_ppg_binary"
#define MAX_FILENAME_LENGTH 128

#define DEBUG_FNAME "/tmp/max86150_logs.txt"

void init_debug(void);
int open_capture_file(char *name);
int close_capture_file();
void close_debug();
void d_print(const char *__format, ...);


#endif /* INCLUDE_FILEWORK_H_ */
