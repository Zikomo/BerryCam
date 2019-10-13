cmake_minimum_required(VERSION 3.7.2)
project(BerryCam)

set(CMAKE_CXX_STANDARD 14)

add_executable(${PROJECT_NAME}
        AudioCaptureDevice.h
        Broadcaster.h
        Camera.h
        Encoder.h
        H264Encoder.cpp
        H264Encoder.h
        main.cpp
        AlsaAudioCaptureDevice.cpp
        AlsaAudioCaptureDevice.h
        MmalSingleImageEncoder.cpp
        MmalSingleImageEncoder.h
        RaspberryPiCamera.cpp
        RaspberryPiCamera.h
        UdpBroadcaster.cpp
        UdpBroadcaster.h
        Utilities.cpp
        Utilities.h)

find_package(Boost REQUIRED COMPONENTS system program_options)
find_package(ALSA REQUIRED)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
include_directories(/opt/vc/include/)
FIND_LIBRARY(FFMPEG_AVCODEC REQUIRED  avcodec)
FIND_LIBRARY(FFMPEG_AVUTIL  REQUIRED avutil)
FIND_LIBRARY(bcm_host_LIBS NAMES bcm_host PATHS /opt/vc/lib)
FIND_LIBRARY(mmalcore_LIBS NAMES mmal_core PATHS /opt/vc/lib)
FIND_LIBRARY(mmalutil_LIBS NAMES mmal_util PATHS /opt/vc/lib)
FIND_LIBRARY(mmal_LIBS NAMES mmal PATHS /opt/vc/lib)

INCLUDE_DIRECTORIES(
        ${FFMPEG_AVCODEC_INCLUDE_DIRS}
        ${FFMPEG_AVUTIL_INCLUDE_DIRS}
        ${Boost_INCLUDE_DIRS}
)
LINK_DIRECTORIES(
        ${FFMPEG_AVCODEC_LIBRARY_DIRS}
        ${FFMPEG_AVUTIL_LIBRARY_DIRS}
        ${Boost_LIBRARY_DIRS}
)
target_link_libraries(${PROJECT_NAME}
        ${FFMPEG_AVCODEC}
        ${FFMPEG_AVUTIL}
        ${ALSA_LIBRARY}
        ${bcm_host_LIBS}
        ${Boost_LIBRARIES}
        ${mmalcore_LIBS}
        ${mmalutil_LIBS}
        ${mmal_LIBS})



