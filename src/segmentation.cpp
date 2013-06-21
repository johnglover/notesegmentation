#include "segmentation.h"

using namespace notesegmentation;

RTSegmentation::RTSegmentation() : MIN_ONSET_GAP_MS(200) {
    _odf = new PeakAmpDifferenceODF();
    _od = new RTOnsetDetection();

    _freqs = NULL;
    _window = NULL;
    _fft_in = NULL;
    _fft_out = NULL;

    _sampling_rate = 44100;
    hop_size(512);
    frame_size(512);

    _min_onset_gap = (_sampling_rate * MIN_ONSET_GAP_MS) / 1000;
    _current_onset_gap = 0;
    _current_region = NONE;

    _peak_rms = 0.f;
    _peak_amp = 0.f;

    _n_prev_odf_values = 2;
    _n_prev_amp_values = 3;
    _n_prev_amp_ma_values = 5;
    _n_amp_mean_values = 3;
    _n_prev_centroid_values = 2;
    _prev_odf_values = new sample[_n_prev_odf_values];
    _prev_amp_values = new sample[_n_prev_amp_values];
    _prev_amp_ma_values = new sample[_n_prev_amp_ma_values];
    _prev_centroid_values = new sample[_n_prev_centroid_values];
    memset(_prev_odf_values, 0.0, sizeof(sample) * _n_prev_odf_values);
    memset(_prev_amp_values, 0.0, sizeof(sample) * _n_prev_amp_values);
    memset(_prev_amp_ma_values, 0.0, sizeof(sample) * _n_prev_amp_ma_values);
    memset(_prev_centroid_values, 0.0,
           sizeof(sample) * _n_prev_centroid_values);

    _n_frames = 0;
    _prev_centroid_cma = 0.f;
}

RTSegmentation::~RTSegmentation() {
    if(_odf) {
        delete _odf;
        _odf = NULL;
    }
    if(_od) {
        delete _od;
        _od = NULL;
    }
    if(_window) {
        delete [] _window;
        _window = NULL;
    }
    if(_freqs) {
        delete [] _freqs;
        _freqs = NULL;
    }

    if(_fft_in) {
        fftw_free(_fft_in);
        _fft_in = NULL;
    }
    if(_fft_out) {
        fftw_free(_fft_out);
        _fft_out = NULL;
    }
    fftw_destroy_plan(_plan);

    if(_prev_odf_values) {
        delete [] _prev_odf_values;
        _prev_odf_values = NULL;
    }
    if(_prev_amp_values) {
        delete [] _prev_amp_values;
        _prev_amp_values = NULL;
    }
    if(_prev_amp_ma_values) {
        delete [] _prev_amp_ma_values;
        _prev_amp_ma_values = NULL;
    }
    if(_prev_centroid_values) {
        delete [] _prev_centroid_values;
        _prev_centroid_values = NULL;
    }
}

// Clear all stored values so ready for new note
void RTSegmentation::reset() {
    _odf->reset();
}

int RTSegmentation::frame_size() {
    return _frame_size;
}

void RTSegmentation::frame_size(int frame_size) {
    _frame_size = frame_size;
    _num_bins = (_frame_size / 2) + 1;

    if(_freqs) {
        delete [] _freqs;
    }
    _freqs = new sample[_num_bins];
    sample base_freq = (sample)_sampling_rate / (sample)_frame_size;
    for(int bin = 0; bin < _num_bins; bin++) {
        _freqs[bin] = bin * base_freq;
    }

    if(_window) {
        delete [] _window;
    }
    _window = new sample[_frame_size];
    windows::hamming(_frame_size, _window);

    if(_fft_in) {
        fftw_free(_fft_in);
    }
    _fft_in = (sample*) fftw_malloc(sizeof(sample) * _frame_size);

    if(_fft_out) {
        fftw_free(_fft_out);
    }
    _fft_out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * _num_bins);

    fftw_destroy_plan(_plan);
	_plan = fftw_plan_dft_r2c_1d(_frame_size, _fft_in,
                                 _fft_out, FFTW_ESTIMATE);

    _odf->set_frame_size(_frame_size);
}

int RTSegmentation::hop_size() {
    return _hop_size;
}

void RTSegmentation::hop_size(int hop_size) {
    _hop_size = hop_size;
    _odf->set_hop_size(hop_size);
}

// Return the spectral centroid for a given frame of audio
sample RTSegmentation::spectral_centroid(int n, sample* audio) {
    if(n != _frame_size) {
        throw Exception(std::string("Size of spectral centroid not ") +
                        std::string("equal to segmentation frame size"));
        return 0.f;
    }

    sample centroid = 0.f;
    sample freqs_amps_sum = 0.f;
    sample amps_sum = 0.f;
    sample amp = 0.f;

    memcpy(_fft_in, &audio[0], sizeof(sample) * _frame_size);
    windows::window(_frame_size, _window, _fft_in);
    fftw_execute(_plan);

    for(int bin = 0; bin < _num_bins; bin++) {
        amp = sqrt((_fft_out[bin][0] * _fft_out[bin][0]) +
                   (_fft_out[bin][1] * _fft_out[bin][1]));
        amps_sum += amp;
        freqs_amps_sum += _freqs[bin] * amp;
    }

    if(amps_sum > 0.f) {
        centroid = freqs_amps_sum / amps_sum;
    }

    return centroid;
}

int RTSegmentation::segment(int n, sample* audio) {
    if(n != _frame_size) {
        throw Exception(std::string("Size of audio frame not ") +
                        std::string("equal to segmentation frame size"));
        return NONE;
    }

    sample odf_value = _odf->process_frame(_frame_size, audio);
    sample centroid = spectral_centroid(_frame_size, audio);
    sample centroid_cma = util::cumulative_moving_average(
        _n_frames, centroid, _prev_centroid_cma
    );

    sample rms_value = rms(_frame_size, audio);
    util::rotate(_n_prev_amp_values, _prev_amp_values, rms_value);

    sample rms_ma_value = mean(_n_amp_mean_values, _prev_amp_values);

    for(int i = 0; i < _frame_size; i++) {
        if(fabs(audio[i]) > _peak_amp) {
            _peak_amp = fabs(audio[i]);
        }
    }
    if(rms_ma_value > _peak_rms) {
        _peak_rms = rms_ma_value;
    }

    bool is_odf_minima = util::is_minima(odf_value, 1, _prev_odf_values);
    bool is_rms_maxima = util::is_maxima(rms_ma_value, 1, _prev_amp_ma_values);

    util::rotate(_n_prev_odf_values, _prev_odf_values, odf_value);
    util::rotate(_n_prev_amp_ma_values, _prev_amp_ma_values, rms_ma_value);

    // update onset and offset, as they can only last for 1 frame
    if(_current_region == ONSET) {
        _current_region = ATTACK;
    }
    else if(_current_region == OFFSET) {
        _current_region = NONE;
    }

    _current_onset_gap += _frame_size;

    if(_od->is_onset(odf_value)) {
        // if not too close to a previous onset, start a new note attack
        if(_current_onset_gap >= _min_onset_gap) {
            _current_onset_gap = 0;
            _current_region = ONSET;
        }
    }

    if(_current_region == ATTACK) {
        // transient/attack: from onset until next minima in odf,
        // unless amp local max reached first
        if(is_odf_minima || is_rms_maxima) {
            _current_region = SUSTAIN;
        }
    }
    else if(_current_region == SUSTAIN) {
        // release: 3 consecutive frames with decreasing energy and
        //          spectral centroid below average
        if((rms_ma_value <= 0.8 * _peak_rms &&
            util::decreasing(_n_prev_amp_ma_values, _prev_amp_ma_values) &&
            centroid < centroid_cma) ||
           (rms_ma_value <= 0.33 * _peak_rms)) {
            _current_region = RELEASE;
        }
    }
    else if(_current_region == RELEASE) {
        // offset: point (after sustain) at which envelope <=-60db
        //         below peak value
        sample offset_threshold = 0.001 * _peak_amp;
        if(rms_value <= offset_threshold) {
            _current_region = OFFSET;
        }
    }

    _n_frames += 1;
    _prev_centroid_cma = centroid_cma;

    return _current_region;
}
