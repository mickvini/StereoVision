cmake_minimum_required(VERSION 3.8)

# Включение политики для MSVC, только если используете MSVC
if (POLICY CMP0141)
    cmake_policy(SET CMP0141 NEW)
endif()

# Название проекта
project("StereoCameraMain")

# Путь к OpenCV (обновите пути под вашу среду)
set(OpenCV_DIR "C:/Users/romanyuk_ei/Documents/SDK/opencv-4.10.0/build/")

# Найдите OpenCV
find_package(OpenCV REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})
include_directories("C:/Users/romanyuk_ei/Documents/SDK/RTSP_dll_64bit/libffmpeghelper/include/")
# Источники проекта
add_executable(${PROJECT_NAME}    
   "src/${PROJECT_NAME}.cpp"   
   "src/StereoVision.h" "src/StereoVision.cpp"
 "src/Encode.h" "src/Encode.cpp")

# Линковка OpenCV
target_link_libraries(${PROJECT_NAME}
    ${OpenCV_LIBS}
    "C:/Users/romanyuk_ei/Documents/SDK/RTSP_dll_64bit/libffmpeghelper/lib/x64/avcodec.lib"
    "C:/Users/romanyuk_ei/Documents/SDK/RTSP_dll_64bit/libffmpeghelper/lib/x64/avformat.lib"
    "C:/Users/romanyuk_ei/Documents/SDK/RTSP_dll_64bit/libffmpeghelper/lib/x64/avutil.lib"
    "C:/Users/romanyuk_ei/Documents/SDK/RTSP_dll_64bit/libffmpeghelper/lib/x64/swscale.lib"
)

# Настройка стандарта C++
if (CMAKE_VERSION VERSION_GREATER 3.12)
    set_property(TARGET StereoCameraMain PROPERTY CXX_STANDARD 20)
endif()