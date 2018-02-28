cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

project(pcd_read)

add_subdirectory(../dlib dlib_build)

find_package(PCL 1.2 REQUIRED)
find_package(OpenCV REQUIRED)
#find_package(dlib REQUIRED)
#include_directories(${dlib_INCLUDE_DIRS})
#link_directories(${dlib_LIBRARY_DIRS})


include_directories(${PCL_INCLUDE_DIRS})
link_directories(${PCL_LIBRARY_DIRS})
add_definitions(${PCL_DEFINITIONS})

include_directories(/usr/local/include/stk)
link_directories(/usr/local/lib)

#TARGET_LINK_LIBRARIES(kinectueye stk):
#add_library(stk)

add_executable (stktest mytest.cpp)
add_executable (pcd_read pcd_read.cpp)
add_executable (cvtest opencvtest.cpp)
add_executable (pcd2png pcd2png.cpp)
target_link_libraries (pcd_read ${PCL_LIBRARIES})
target_link_libraries (pcd2png ${PCL_LIBRARIES})
target_link_libraries (stktest stk)
target_link_libraries(cvtest dlib::dlib)

if (OpenCV_FOUND)
  include_directories(${OpenCV_INCLUDE_DIRS})
  target_link_libraries (cvtest ${OpenCV_LIBRARIES})
endif (OpenCV_FOUND)
