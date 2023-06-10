/*
 * filename: signalwork.h
 */

#ifndef INCLUDE_SIGNALWORK_H_
#define INCLUDE_SIGNALWORK_H_

int start_max86150_timer(uint32_t samp_freq);
int stop_max86150_timer(void);
int register_term_signal(void);
int get_sigint_status(void);

#endif /* INCLUDE_SIGNALWORK_H_ */
