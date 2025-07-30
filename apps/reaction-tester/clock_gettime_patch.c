#include <time.h>
#include "pico/stdlib.h"

int clock_gettime(clockid_t clk_id, struct timespec *tp) {
    if (tp == NULL) return -1;

    absolute_time_t now = get_absolute_time();
    uint64_t us = to_us_since_boot(now);

    tp->tv_sec = us / 1000000;
    tp->tv_nsec = (us % 1000000) * 1000;

    return 0;
}