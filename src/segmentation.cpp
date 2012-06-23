#include "segmentation.h"

GLT::GLT() : MIN_ONSET_GAP_MS(200) {
    hop_size = 512;
    frame_size = 512;
    sampling_rate = 44100;
    num_bins = (frame_size / 2) + 1;

    min_onset_gap = (sampling_rate * MIN_ONSET_GAP_MS) / 1000;
    current_onset_gap = 0;

    current_region = NONE;

    odf = new PeakAmpDifferenceODF();
    odf->set_hop_size(hop_size);
    odf->set_frame_size(frame_size);

    od = new RTOnsetDetection();

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

    n_prev_odf_values = 2;
    n_prev_amp_values = 3;
    n_prev_amp_ma_values = 3;
    n_prev_centroid_values = 2;
    prev_odf_values = new sample[n_prev_odf_values];
    prev_amp_values = new sample[n_prev_amp_values];
    prev_amp_ma_values = new sample[n_prev_amp_ma_values];
    prev_centroid_values = new sample[n_prev_centroid_values];
    memset(prev_odf_values, 0.0, sizeof(sample) * n_prev_odf_values);
    memset(prev_amp_values, 0.0, sizeof(sample) * n_prev_amp_values);
    memset(prev_amp_ma_values, 0.0, sizeof(sample) * n_prev_amp_ma_values);
    memset(prev_centroid_values, 0.0, sizeof(sample) * n_prev_centroid_values);
}

GLT::~GLT() {
    delete odf;
    delete od;

    delete [] window;
    delete [] freqs;

    if(fft_in) fftw_free(fft_in);
    if(fft_out) fftw_free(fft_out);
    fftw_destroy_plan(p);

    delete [] prev_odf_values;
    delete [] prev_amp_values;
    delete [] prev_amp_ma_values;
    delete [] prev_centroid_values;
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
    sample odf_value = odf->process_frame(frame_size, audio);
    sample centroid = spectral_centroid(frame_size, audio);

    sample rms_value = rms(frame_size, audio);
    util::rotate(n_prev_amp_values, prev_amp_values, rms_value);

    sample rms_ma_value = mean(n_prev_amp_values, prev_amp_values);

    // update onset and offset, as they can only last for 1 frame
    if(current_region == ONSET) {
        current_region = ATTACK;
    }
    else if(current_region == OFFSET) {
        current_region = NONE;
    }

    current_onset_gap += frame_size;

    if(od->is_onset(odf_value)) {
        // if not too close to a previous onset, start a new note attack
        if(current_onset_gap >= min_onset_gap) {
            current_onset_gap = 0;
            current_region = ONSET;
        }
    }

    // if in the attack region, check for the start of sustain
    if(current_region == ATTACK) {
        // transient/attack: from onset until next minima in odf,
        // unless amp local max reached first
        if(util::is_minima(odf_value, n_prev_odf_values, prev_odf_values) ||
           util::is_maxima(rms_ma_value, n_prev_amp_ma_values, prev_amp_ma_values)) {
            current_region = SUSTAIN;
        }
    }

    // if in the sustain region, check for the start of release
    else if(current_region == SUSTAIN) {
    }

    // if in the release region, check for the offset
    else if(current_region == RELEASE) {
    }

    util::rotate(n_prev_odf_values, prev_odf_values, odf_value);
    util::rotate(n_prev_amp_ma_values, prev_amp_ma_values, rms_ma_value);

    return current_region;
}
