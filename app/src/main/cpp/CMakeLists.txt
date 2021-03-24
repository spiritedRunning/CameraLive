
cmake_minimum_required(VERSION 3.4.1)


add_subdirectory(librtmp)

add_library(
             native-lib
             SHARED
             native-lib.cpp
            JavaCallHelper.cpp)

target_link_libraries(
                       native-lib
                       log
                       rtmp)



# 图像处理操作 so
include_directories(${CMAKE_SOURCE_DIR}/libyuv/include)
add_subdirectory(libyuv)


add_library(ImageUtils
        SHARED
        ImageUtils.cpp
        )

target_link_libraries(ImageUtils
                    yuv)