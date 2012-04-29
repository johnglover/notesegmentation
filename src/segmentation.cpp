#include "segmentation.h"

GLT::GLT() {
    frame_size = 512;
    sampling_rate = 44100;
    num_bins = (frame_size / 2) + 1;

    freqs = new sample[num_bins];
    sample base_freq = (sample)sampling_rate / (sample)frame_size;
    for(int bin = 0; bin < num_bins; bin++) {
        freqs[bin] = bin * base_freq;
    }

    window = new sample[frame_size];
    windows::hamming(frame_size, window);

    fft_in = (sample*) fftw_malloc(sizeof(sample) * frame_size);
    fft_out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * num_bins);
	p = fftw_plan_dft_r2c_1d(frame_size, fft_in, fft_out, FFTW_ESTIMATE);
}

GLT::~GLT() {
    delete [] window;
    delete [] freqs;
    if(fft_in) fftw_free(fft_in);
    if(fft_out) fftw_free(fft_out);
    fftw_destroy_plan(p);
}

// Return the spectral centroid for a given frame of audio
sample GLT::spectral_centroid(int n, sample* audio) {
    sample centroid = 0.f;
    sample freqs_amps_sum = 0.f;
    sample amps_sum = 0.f;
    sample amp = 0.f;

    memcpy(fft_in, &audio[0], sizeof(sample)*frame_size);
    windows::window(frame_size, window, fft_in);
    fftw_execute(p);

    for(int bin = 0; bin < num_bins; bin++) {
        amp = sqrt((fft_out[bin][0] * fft_out[bin][0]) +
                   (fft_out[bin][1] * fft_out[bin][1]));
        amps_sum += amp;
        freqs_amps_sum += freqs[bin] * amp;
    }

    if(amps_sum > 0.f) {
        centroid = freqs_amps_sum / amps_sum;
    }

    return centroid;
}

int GLT::segment(int n, sample* audio) {
    return NONE;
}
