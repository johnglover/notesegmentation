#include "util.h"

// Return the cumulative moving average, given a total of n previous values
sample util::cumulative_moving_average(int n, sample current, sample prev) {
    return prev + ((current - prev) / (n + 1));
}

// Set each point in cma to be the cumulative moving average of the
// corresponding point in signal.
void util::cumulative_moving_average_frame(int n, sample* signal, sample* cma,
                                           int n_prev, sample prev) {
    sample p = prev;
    for(int i = 0; i < n; i ++) {
        cma[i] = util::cumulative_moving_average(n_prev + i, signal[i], p);
        p = cma[i];
    }
}
