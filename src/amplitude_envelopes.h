#ifndef _AMPLITUDEENVELOPES_H
#define _AMPLITUDEENVELOPES_H

#include <math.h>

typedef double sample;

// Return the mean value of n samples
sample mean(int n, sample* a);

// Return the root mean square of the samples in a given frame of audio.
sample rms(int n, sample* audio);

// Return the root mean square of the samples in a given frame of audio.
//
// Uses the average value of the current rms and the previous n_prev
// values.
sample rms(int n, sample* audio, int n_prev, sample* prev);

#endif
