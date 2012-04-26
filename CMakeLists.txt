cmake_minimum_required(VERSION 2.6)
project(notesegmentation)

set(source_files src/segmentation.cpp 
                 src/util.cpp
                 src/amplitude_envelopes.cpp)

set(include_files src/segmentation.h 
                  src/util.h
                  src/amplitude_envelopes.h)

include_directories(src)
add_library(notesegmentation SHARED ${source_files})
target_link_libraries(notesegmentation modal)

install(TARGETS notesegmentation LIBRARY DESTINATION lib)
install(FILES ${include_files} DESTINATION include/notesegmentation)
