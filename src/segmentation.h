#ifndef _NOTESEGMENTATION_H
#define _NOTESEGMENTATION_H

#include "modal/detectionfunctions.h"
#include "modal/onsetdetection.h"
#include "util.h"

typedef double sample;

enum NoteRegion {ATTACK, SUSTAIN, RELEASE, NONE};

class GLT {
    private:
        PeakAmpDifferenceODF odf;

    public:
        int segment(int n, sample* audio);
};

#endif
