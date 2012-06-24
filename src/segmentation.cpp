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

    peak_amp = 0.f;

    n_prev_odf_values = 2;
    n_prev_amp_values = 3;
    n_prev_amp_ma_values = 5;
    n_amp_mean_values = 3;
    n_prev_centroid_values = 2;
    prev_odf_values = new sample[n_prev_odf_values];
    prev_amp_values = new sample[n_prev_amp_values];
    prev_amp_ma_values = new sample[n_prev_amp_ma_values];
    prev_centroid_values = new sample[n_prev_centroid_values];
    memset(prev_odf_values, 0.0, sizeof(sample) * n_prev_odf_values);
    memset(prev_amp_values, 0.0, sizeof(sample) * n_prev_amp_values);
    memset(prev_amp_ma_values, 0.0, sizeof(sample) * n_prev_amp_ma_values);
    memset(prev_centroid_values, 0.0, sizeof(sample) * n_prev_centroid_values);

    n_frames = 0;
    prev_centroid_cma = 0.f;
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

// Clear all stored values so ready for new note
void GLT::reset() {
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
    sample centroid_cma = util::cumulative_moving_average(n_frames, centroid, prev_centroid_cma);

    sample rms_value = rms(frame_size, audio);
    util::rotate(n_prev_amp_values, prev_amp_values, rms_value);

    sample rms_ma_value = mean(n_amp_mean_values, prev_amp_values);

    if(rms_ma_value > peak_amp) {
        peak_amp = rms_ma_value;
    }

    bool is_odf_minima = util::is_minima(odf_value, 1, prev_odf_values);
    bool is_rms_maxima = util::is_maxima(rms_ma_value, 1, prev_amp_ma_values);

    util::rotate(n_prev_odf_values, prev_odf_values, odf_value);
    util::rotate(n_prev_amp_ma_values, prev_amp_ma_values, rms_ma_value);

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
        if(is_odf_minima || is_rms_maxima) {
            current_region = SUSTAIN;
        }
    }

    // if in the sustain region, check for the start of release
    else if(current_region == SUSTAIN) {
        // release: 3 consecutive frames with decreasing energy and
        //          spectral centroid below average
        if((rms_ma_value <= 0.8 * peak_amp &&
            util::decreasing(n_prev_amp_ma_values, prev_amp_ma_values) &&
            centroid < centroid_cma) ||
           (rms_ma_value <= 0.33 * peak_amp)) {
            current_region = RELEASE;
        }
    }

    // if in the release region, check for the offset
    else if(current_region == RELEASE) {
    }

    n_frames += 1;
    prev_centroid_cma = centroid_cma;

    return current_region;
}
