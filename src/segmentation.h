#ifndef NOTESEGMENTATION_SEGMENTATION_H
#define NOTESEGMENTATION_SEGMENTATION_H

#include <math.h>
#include <fftw3.h>
#include "modal/detectionfunctions.h"
#include "modal/onsetdetection.h"
#include "window.h"
#include "util.h"
#include "amplitude_envelopes.h"
#include "exceptions.h"

namespace notesegmentation
{


typedef double sample;

enum NoteRegion {NONE, ONSET, ATTACK, SUSTAIN, RELEASE, OFFSET};


class RTSegmentation {
    private:
        const int MIN_ONSET_GAP_MS;  // in ms
        int _min_onset_gap;  // in samples
        int _current_onset_gap;  // in samples

        int _current_region;

        PeakAmpDifferenceODF* _odf;
        RTOnsetDetection* _od;

        int _frame_size;
        int _hop_size;
        int _sampling_rate;
        int _num_bins;

        sample* _freqs;
        sample* _window;
        sample* _fft_in;
        fftw_complex* _fft_out;
        fftw_plan _plan;

        sample _peak_rms;  // largest RMS amplitude value in a note
        sample _peak_amp;  // largest amplitude value in a note

        // values used to calculate local minima/maxima
        int _n_prev_odf_values;
        int _n_prev_amp_values;
        int _n_prev_amp_ma_values;
        int _n_amp_mean_values;
        int _n_prev_centroid_values;
        sample* _prev_odf_values;
        sample* _prev_amp_values;
        sample* _prev_amp_ma_values;
        sample* _prev_centroid_values;

        // values needed for centroid cumulative moving average calcuation
        long _n_frames;
        sample _prev_centroid_cma;

    public:
        RTSegmentation();
        ~RTSegmentation();

        void reset();
        int frame_size();
        void frame_size(int frame_size);
        int hop_size();
        void hop_size(int hop_size);
        sample spectral_centroid(int n, sample* audio);
        int segment(int n, sample* audio);
};


} // end of namespace notesegmentation

#endif
