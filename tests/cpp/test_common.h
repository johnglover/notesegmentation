#ifndef NOTESEGMENTATION_TEST_COMMON_H
#define NOTESEGMENTATION_TEST_COMMON_H

#include <iostream>
#include <vector>
#include <sndfile.hh>

#include "exceptions.h"

namespace notesegmentation
{

typedef double sample;
static const double PRECISION = 0.001;
static const char* TEST_AUDIO_FILE = "../tests/audio/clarinet.wav";

} // end of namespace notesegmentation

#endif
