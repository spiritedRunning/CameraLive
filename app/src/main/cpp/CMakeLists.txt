
cmake_minimum_required(VERSION 3.4.1)

include_directories(${CMAKE_SOURCE_DIR}/librtmp)
add_subdirectory(librtmp)

add_library(
             native-lib
             SHARED
             native-lib.cpp
            JavaCallHelper.cpp
            VideoChannel.cpp
            AudioChannel.cpp)


include_directories(${CMAKE_SOURCE_DIR}/x264/${ANDROID_ABI}/include)
include_directories(${CMAKE_SOURCE_DIR}/faac/${ANDROID_ABI}/include)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -L${CMAKE_SOURCE_DIR}/x264/${ANDROID_ABI}/lib")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -L${CMAKE_SOURCE_DIR}/faac/${ANDROID_ABI}/lib")

target_link_libraries(native-lib
                       log
                       rtmp
                        faac
                        x264)



include_directories(${CMAKE_SOURCE_DIR}/libyuv/include)
add_subdirectory(libyuv)

# 图像处理操作 so
add_library(ImageUtils
        SHARED
        ImageUtils.cpp
        )

target_link_libraries(ImageUtils
                    yuv)