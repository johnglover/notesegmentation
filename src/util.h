#ifndef _UTIL_H
#define _UTIL_H

typedef double sample;

namespace util
{
// Set the first value in a to new_value and rotate the rest of the values
void rotate(int n, sample* a, sample new_value);

// Return the cumulative moving average, given a total of n previous values
sample cumulative_moving_average(long n, sample current, sample prev);

// Set each point in cma to be the cumulative moving average of the
// corresponding point in signal.
void cumulative_moving_average_frame(int n, sample* signal, sample* cma,
                                     int n_prev, sample prev);

// Return true if previous sample is a local minima
// (1 sample delay to check before and after)
bool is_minima(sample current, int n_prev, sample* prev);

// Return true if previous sample is a local maxima
// (1 sample delay to check before and after)
bool is_maxima(sample current, int n_prev, sample* prev);

// Return true if s is a decreasing signal
// Signal goes from 0..n, where 0 is the most recent value.
bool decreasing(int n, sample* signal);

} // end of namespace util

#endif
