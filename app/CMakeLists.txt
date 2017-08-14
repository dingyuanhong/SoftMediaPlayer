# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.



add_library( media_controller-lib SHARED  src/main/cpp/media_controller-lib.cpp
                                          src/main/cpp/EvoInterface/EvoMediaSource.cpp
                                          src/main/cpp/EvoInterface/EvoVideoConvert.cpp
                                          src/main/cpp/EvoInterface/VideoDecoder.cpp
                                          src/main/cpp/MediaDecode.cpp
                                          src/main/cpp/EvoInterface/sei_packet.cpp
)

add_library( libevoffmpeg SHARED IMPORTED )
set_target_properties( libevoffmpeg PROPERTIES IMPORTED_LOCATION
                       ${CMAKE_SOURCE_DIR}/libs/${ANDROID_ABI}/libevoffmpeg.so)


include_directories( libs/include )
#include_directories( libs/includeOld )

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( log-lib log )

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries(media_controller-lib
                        libevoffmpeg

                        ${log-lib} )

#avformat
#avcodec
#avutil
#avfilter
#swresample
#swscale

#libavformat
#libavcodec
#libavutil
#libavfilter
#libswresample
#libswscale

#libavformat-56
#libavcodec-56
#libavutil-54
#libavfilter-5
#libswresample-1
#libswscale-3