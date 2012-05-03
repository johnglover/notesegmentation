#include "window.h"

void windows::window(int window_size, sample* window, sample* audio) {
    for(int i = 0; i < window_size; i++) {
        audio[i] *= window[i];
    }
}

void windows::hamming(int window_size, sample* window) {
	for(int i = 0; i < window_size; i++) {
		window[i] = 0.54 - (0.46 * cos(2.0*M_PI*i/(window_size-1)));
	}
}

void windows::hann(int window_size, sample* window) {
	for(int i = 0; i < window_size; i++) {
		window[i] = 0.5 * (1.0 - cos(2.0*M_PI*i/(window_size-1)));
	}
}
