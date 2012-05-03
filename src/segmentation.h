#ifndef _NOTESEGMENTATION_H
#define _NOTESEGMENTATION_H

#include <fftw3.h>
#include "modal/detectionfunctions.h"
#include "modal/onsetdetection.h"
#include "window.h"
#include "util.h"

typedef double sample;

enum NoteRegion {ATTACK, SUSTAIN, RELEASE, NONE};

class GLT {
    private:
        PeakAmpDifferenceODF* odf;
        int frame_size;
        int sampling_rate;
        int num_bins;
        sample* freqs;
        sample* window;
        sample* fft_in;
        fftw_complex* fft_out;
        fftw_plan p;

    public:
        GLT();
        ~GLT();
        sample spectral_centroid(int n, sample* audio);
        int segment(int n, sample* audio);
};

#endif
