#ifndef _WINDOWS_H
#define _WINDOWS_H

#include <math.h>

typedef double sample;

namespace windows
{

void window(int window_size, sample* window, sample* audio);
void hann(int window_size, sample* window);
void hamming(int window_size, sample* window);

} // end of namespace window

#endif
