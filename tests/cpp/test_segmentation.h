#ifndef NOTESEGMENTATION_TEST_SEGMENTATION_H
#define NOTESEGMENTATION_TEST_SEGMENTATION_H

#include <cppunit/extensions/HelperMacros.h>

#include "exceptions.h"
#include "segmentation.h"
#include "test_common.h"

namespace notesegmentation
{


class TestSegmentation : public CPPUNIT_NS::TestCase {
    CPPUNIT_TEST_SUITE(TestSegmentation);
    CPPUNIT_TEST(test_basic);
    CPPUNIT_TEST(test_change_hop_frame_sizes);
    CPPUNIT_TEST_SUITE_END();

protected:
    SndfileHandle _sf;
    RTSegmentation _ns;
    
    void test_basic();
    void test_change_hop_frame_sizes();

public:
    void setUp();
};


} // end of namespace notesegmentation

#endif
