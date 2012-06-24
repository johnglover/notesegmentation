#ifndef _NOTESEGMENTATION_H
#define _NOTESEGMENTATION_H

#include <fftw3.h>
#include "modal/detectionfunctions.h"
#include "modal/onsetdetection.h"
#include "window.h"
#include "util.h"
#include "amplitude_envelopes.h"

typedef double sample;

enum NoteRegion {NONE, ONSET, ATTACK, SUSTAIN, RELEASE, OFFSET};


class GLT {
    private:
        const int MIN_ONSET_GAP_MS;  // in ms
        int min_onset_gap;  // in samples
        int current_onset_gap;  // in samples

        int current_region;

        PeakAmpDifferenceODF* odf;
        RTOnsetDetection* od;

        int frame_size;
        int hop_size;
        int sampling_rate;
        int num_bins;

        sample* freqs;
        sample* window;
        sample* fft_in;
        fftw_complex* fft_out;
        fftw_plan p;

        sample peak_amp;  // largest amplitude value in a note

        // values used to calculate local minima/maxima
        int n_prev_odf_values;
        int n_prev_amp_values;
        int n_prev_amp_ma_values;
        int n_amp_mean_values;
        int n_prev_centroid_values;
        sample* prev_odf_values;
        sample* prev_amp_values;
        sample* prev_amp_ma_values;
        sample* prev_centroid_values;

        // values needed for centroid cumulative moving average calcuation
        long n_frames;
        sample prev_centroid_cma;

    public:
        GLT();
        ~GLT();

        void reset();
        sample spectral_centroid(int n, sample* audio);
        int segment(int n, sample* audio);
};

#endif
