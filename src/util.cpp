#include "util.h"

// Set the first value in a to new_value and rotate the rest of the values
void util::rotate(int n, sample* a, sample new_value) {
    for(int i = n - 1; i > 0; i--) {
        a[i] = a[i - 1];
    }
    a[0] = new_value;
}

// Return the cumulative moving average, given a total of n previous values
sample util::cumulative_moving_average(long n, sample current, sample prev) {
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

// Return true if previous sample is a local minima
// (1 sample delay to check before and after)
bool util::is_minima(sample current, int n_prev, sample* prev) {
    bool result = true;

    for(int i = 1; i < n_prev; i++) {
        if(prev[0] >= prev[i]) {
            result = false;
            break;
        }
    }

    if(result && (prev[0] >= current)) {
        result = false;
    }

    return result;
}

// Return true if previous sample is a local maxima
// (1 sample delay to check before and after)
bool util::is_maxima(sample current, int n_prev, sample* prev) {
    bool result = true;

    for(int i = 1; i < n_prev; i++) {
        if(prev[0] <= prev[i]) {
            result = false;
            break;
        }
    }

    if(result && (prev[0] <= current)) {
        result = false;
    }

    return result;
}

// Return true if s is a decreasing signal.
// Signal goes from 0..n, where 0 is the most recent value.
bool util::decreasing(int n, sample* signal) {
    for(int i = 0; i < n - 1; i++) {
        if(signal[i] >= signal[i + 1]) {
            return false;
        }
    }

    return true;
}
