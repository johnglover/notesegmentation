#ifndef NOTESEGMENTATION_WINDOWS_H
#define NOTESEGMENTATION_WINDOWS_H

#include <math.h>

namespace windows
{

typedef double sample;

void window(int window_size, sample* window, sample* audio);
void hann(int window_size, sample* window);
void hamming(int window_size, sample* window);

} // end of namespace window

#endif
