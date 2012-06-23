#include "amplitude_envelopes.h"

// Return the mean value of n samples
sample mean(int n, sample* a) {
    if(n <= 0) {
        return 0.f;
    }

    sample m = 0.0;
    for(int i = 0; i < n; i++) {
        m += a[i];
    }
    return m / n;
}

// Return the root mean square of the samples in a given frame of audio
sample rms(int n, sample* audio) {
    if(n <= 0) {
        return 0.0;
    }

    sample s = 0.0;
    for(int i = 0; i < n; i++) {
        s += audio[i] * audio[i];
    }
    return sqrt(s / n);
}

// Return the root mean square of the samples in a given frame of audio.
//
// Uses the average value of the current rms and the previous n_prev
// values.
sample rms(int n, sample* audio, int n_prev, sample* prev) {
    if(n <= 0) {
        return 0.0;
    }

    sample s = 0.0;
    for(int i = 0; i < n; i++) {
        s += audio[i] * audio[i];
    }
    s = sqrt(s/n);

    for(int i = n_prev - 1; i > 0; i--) {
        prev[i] = prev[i - 1];
    }
    prev[0] = s;

    return mean(n_prev, prev);
}
