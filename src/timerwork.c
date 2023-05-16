/*
 * filename: timerwork.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <filework.h>
#include <peripheral.h>
#include <timerwork.h>

#define UNUSED(x) ((void)x)

static void set_timer_specs(struct sigevent *s_event,
                            struct sigaction *s_action,
                            struct itimerspec *its,
                            uint32_t period_ns);
static uint32_t sampling_freq_2_period_ns(uint32_t samp_freq);
static void max86150_timer_action(int sig, siginfo_t *si, void *uc);

static timer_t read_periodic_timer;


int start_max86150_timer(uint32_t samp_freq) {
    struct sigevent   s_event  = {0};
    struct sigaction  s_action = {0};
    struct itimerspec its      = {0};

    set_timer_specs(&s_event, &s_action, &its, sampling_freq_2_period_ns(samp_freq));

    if (timer_create(CLOCK_REALTIME, &s_event, &read_periodic_timer)) {
        d_print("%s: cannot create timer - %s\n", __func__, strerror(errno));
        return -1;
    }

    sigemptyset(&s_action.sa_mask);

    if (sigaction(SIGRTMIN, &s_action, NULL)) {
        d_print("%s: cannot register sigaction - %s\n", __func__, strerror(errno));
        return -1;
    }

    if (timer_settime(read_periodic_timer, 0, &its, NULL)) {
        d_print("%s: cannot start timer - %s\n", __func__, strerror(errno));
        return -1;
    }

    return 0;
}


static void set_timer_specs(struct sigevent *s_event,
                            struct sigaction *s_action,
                            struct itimerspec *its,
                            uint32_t period_ns)
{
    its->it_value.tv_sec     = 0;
    its->it_value.tv_nsec    = period_ns << 3; /* Timer must be called every 8 sampling periods */
    its->it_interval.tv_sec  = 0;
    its->it_interval.tv_nsec = period_ns << 3;

    s_event->sigev_notify = SIGEV_SIGNAL;
    s_event->sigev_signo = SIGRTMIN;
    s_event->sigev_value.sival_ptr = NULL;

    s_action->sa_flags = SA_SIGINFO;
    s_action->sa_sigaction = max86150_timer_action;
}


static uint32_t sampling_freq_2_period_ns(uint32_t samp_freq) {
    switch (samp_freq) {
        case 10:
            return 100000000;
            break;
        case 20:
            return 50000000;
            break;
        case 50:
            return 20000000;
            break;
        case 84:
            return 11904762;
            break;
        case 100:
            return 10000000;
            break;
        case 200:
            return 5000000;
            break;
        case 400:
            return 2500000;
            break;
        case 800:
            return 1250000;
            break;
        case 1000:
            return 1000000;
            break;
        case 1600:
            return 625000;
            break;
        case 3200:
            return 312500;
            break;
        default:
            return 0;
    }
}


static void max86150_timer_action(int sig, siginfo_t *si, void *uc) {
    UNUSED(sig);
    UNUSED(si);
    UNUSED(uc);

    printf("%s: stub print\n", __func__);

}
