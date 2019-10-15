cmake_minimum_required(VERSION 3.7.2)
project(BerryCam)

set(CMAKE_CXX_STANDARD 14)

add_executable(${PROJECT_NAME}
        Broadcaster.h
        Camera.h
        Encoder.h
        H264Encoder.cpp
        H264Encoder.h
        main.cpp
        MmalSingleImageEncoder.cpp
        MmalSingleImageEncoder.h
        RaspberryPiCamera.cpp
        RaspberryPiCamera.h
        UdpBroadcaster.cpp
        UdpBroadcaster.h
        Utilities.cpp
        Utilities.h
        BerryCamHeader.h)

find_package(Boost REQUIRED COMPONENTS system program_options)

#This is the real build on the Raspberry Pi
if(${CMAKE_SYSTEM_PROCESSOR} MATCHES arm*)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
    include_directories(/opt/vc/include/)
    FIND_LIBRARY(avcodec_LIBS avcodec)
    FIND_LIBRARY(avutil_LIBS NAMES avutil)
    FIND_LIBRARY(bcm_host_LIBS NAMES bcm_host PATHS /opt/vc/lib)
    FIND_LIBRARY(mmalcore_LIBS NAMES mmal_core PATHS /opt/vc/lib)
    FIND_LIBRARY(mmalutil_LIBS NAMES mmal_util PATHS /opt/vc/lib)
    FIND_LIBRARY(mmal_LIBS NAMES mmal PATHS /opt/vc/lib)
    link_directories( ${Boost_LIBRARY_DIRS})
    target_link_libraries(${PROJECT_NAME}
            ${avcodec_LIBS}
            ${avutil_LIBS}
            ${bcm_host_LIBS}
            ${Boost_LIBRARIES}
            ${mmalcore_LIBS}
            ${mmalutil_LIBS}
            ${mmal_LIBS}
)
else()
    #This is here simply for building on non Raspberry Pi systems.
    include_directories(/usr/local/Cellar/ffmpeg/4.1.1/include  )
    include_directories(dependencies)
endif()



