cmake_minimum_required(VERSION 2.6)
project(notesegmentation)

set(source_files src/segmentation.cpp 
                 src/window.cpp
                 src/util.cpp
                 src/amplitude_envelopes.cpp
                 src/exceptions.cpp)

set(include_files src/segmentation.h 
                  src/window.h
                  src/util.h
                  src/amplitude_envelopes.h
                  src/exceptions.h)

set(libs m fftw3 modal)

include_directories(src)
add_library(notesegmentation SHARED ${source_files})
target_link_libraries(notesegmentation ${libs})

install(TARGETS notesegmentation LIBRARY DESTINATION lib)
install(FILES ${include_files} DESTINATION include/notesegmentation)

# ----------------------------------------------------------------------------
# Tests
# ----------------------------------------------------------------------------
if(BUILD_TESTS)
    include_directories(tests)

    set(test_src tests/cpp/tests.cpp
                 tests/cpp/test_segmentation.cpp
                 ${source_files})

    add_executable(tests ${test_src})
    target_link_libraries(tests cppunit sndfile ${libs})
else()
    message("Not building tests. To change run CMake with -D BUILD_TESTS=yes")
endif()
