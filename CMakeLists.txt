cmake_minimum_required(VERSION 3.18)

project(wacomswitcher)

include(FindPkgConfig)
pkg_check_modules(GTK4 REQUIRED gtk4)
pkg_check_modules(XRANDR REQUIRED xrandr)
pkg_check_modules(XLIB REQUIRED x11)

set(CMAKE_CXX_STANDARD 20)

add_executable(${PROJECT_NAME} displays.cpp main.cpp wacom.cpp)
target_include_directories(${PROJECT_NAME} PRIVATE ${GTK4_INCLUDE_DIRS} ${XRANDR_INCLUDE_DIRS} ${XLIB_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${GTK4_LIBRARIES} ${XRANDR_LIBRARIES} ${XLIB_LIBRARIES})