cmake_minimum_required(VERSION 3.7.2)
project(BerryCam)

set(CMAKE_CXX_STANDARD 14)

add_executable(BerryCam main.cpp Broadcaster.cpp Broadcaster.h Utilities.cpp Utilities.h)

find_package(Boost REQUIRED COMPONENTS system)

if(${CMAKE_SYSTEM_PROCESSOR} MATCHES arm*)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
    include_directories(/opt/vc/include/)
    FIND_LIBRARY(avcodec_LIBS avcodec)
    FIND_LIBRARY(avutil_LIBS NAMES avutil)

    FIND_LIBRARY(mmalcore_LIBS NAMES mmal_core PATHS /opt/vc/lib)
    FIND_LIBRARY(mmalutil_LIBS NAMES mmal_util PATHS /opt/vc/lib)
    FIND_LIBRARY(mmal_LIBS NAMES mmal PATHS /opt/vc/lib)
    FIND_LIBRARY(bcm_host_LIBS NAMES bcm_host PATHS /opt/vc/lib)
    IF( (NOT mmal_LIBS ) OR (NOT mmalutil_LIBS) OR (NOT mmalcore_LIBS) OR (NOT bcm_host_LIBS) OR (NOT avcodec_LIBS))
        MESSAGE(FATAL_ERROR "Could not find mmal libraries")
    ENDIF()
    set(REQUIRED_LIBRARIES ${REQUIRED_LIBRARIES} ${mmalcore_LIBS} ${mmalutil_LIBS} ${mmal_LIBS} ${bcm_host_LIBS} ${avcodec_LIBS} ${avutil_LIBS} ${Boost_LIBRARIES})
    link_directories( ${Boost_LIBRARY_DIRS})
    target_link_libraries(BerryCam  ${mmalcore_LIBS} ${mmalutil_LIBS} ${mmal_LIBS} ${bcm_host_LIBS} ${avcodec_LIBS}  ${avutil_LIBS} ${Boost_LIBRARIES})
else()
    include_directories(/usr/local/Cellar/ffmpeg/4.1.1/include  )
    include_directories(dependencies)

ENDIF()

##If opencv installed, creates the libraspicam_cv library module
#FIND_PACKAGE(OpenCV)
#IF(OpenCV_FOUND)
#    include_directories(${OpenCV_INCLUDE_DIRS})
#    SET(LIB_COMPONENTS ${LIB_COMPONENTS} ${PROJECT_NAME}_cv)
#    SET(PROJECT_CV_CREATED_FLAG "YES")
#ENDIF()

