#ifndef _UTIL_H
#define _UTIL_H

typedef double sample;

namespace util
{

// Return the cumulative moving average, given a total of n previous values
sample cumulative_moving_average(int n, sample current, sample prev);

// Set each point in cma to be the cumulative moving average of the
// corresponding point in signal.
void cumulative_moving_average_frame(int n, sample* signal, sample* cma,
                                     int n_prev, sample prev);

} // end of namespace util

#endif
