#include "test_segmentation.h"

using namespace notesegmentation;

void TestSegmentation::setUp() {
    _sf = SndfileHandle(TEST_AUDIO_FILE);

    if(_sf.error() > 0) {
        throw Exception(std::string("Could not open audio file: ") +
                        std::string(TEST_AUDIO_FILE));
    }

    _ns.reset();
}

void TestSegmentation::test_basic() {
    std::vector<sample> audio(_sf.frames(), 0.0);
    _sf.read(&audio[0], (int)_sf.frames());
    
    int hop_size = _ns.hop_size();
    int frame_size = _ns.frame_size();
    std::vector<int> segments;

    for(int i = 0; i <= audio.size() - frame_size; i += hop_size) {
        segments.push_back(_ns.segment(frame_size, &(audio[i]))); 
    }

    CPPUNIT_ASSERT(segments.size() > 0);
    
    for(int i = 0; i < segments.size(); i++) {
        CPPUNIT_ASSERT(segments[i] >= NONE);
        CPPUNIT_ASSERT(segments[i] <= OFFSET);
    }
}

void TestSegmentation::test_change_hop_frame_sizes() {
    std::vector<sample> audio(_sf.frames(), 0.0);
    _sf.read(&audio[0], (int)_sf.frames());
    
    int frame_size = 512;
    int hop_size = 256;
    std::vector<int> segments;

    _ns.frame_size(frame_size);
    _ns.hop_size(hop_size);

    for(int i = 0; i <= audio.size() - frame_size; i += hop_size) {
        segments.push_back(_ns.segment(frame_size, &(audio[i]))); 
    }

    CPPUNIT_ASSERT(segments.size() > 0);
    
    for(int i = 0; i < segments.size(); i++) {
        CPPUNIT_ASSERT(segments[i] >= NONE);
        CPPUNIT_ASSERT(segments[i] <= OFFSET);
    }

    frame_size = 256;
    hop_size = 64;
    segments.clear();

    _ns.reset();
    _ns.frame_size(frame_size);
    _ns.hop_size(hop_size);

    for(int i = 0; i <= audio.size() - frame_size; i += hop_size) {
        segments.push_back(_ns.segment(frame_size, &(audio[i]))); 
    }

    CPPUNIT_ASSERT(segments.size() > 0);
    
    for(int i = 0; i < segments.size(); i++) {
        CPPUNIT_ASSERT(segments[i] >= NONE);
        CPPUNIT_ASSERT(segments[i] <= OFFSET);
    }
}
