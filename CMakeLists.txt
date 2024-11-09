cmake_minimum_required(VERSION 3.18)

project(wacomswitcher)

option(USE_ADWAITA "use adwaita" OFF)

include(FindPkgConfig)
pkg_check_modules(GTK4 REQUIRED gtk4)
pkg_check_modules(XRANDR REQUIRED xrandr)
pkg_check_modules(XLIB REQUIRED x11)
set(incl
    ${GTK4_INCLUDE_DIRS}
    ${XLIB_INCLUDE_DIRS}
    ${XRANDR_INCLUDE_DIRS}
)
set(libs
    ${GTK4_LIBRARIES}
    ${XLIB_LIBRARIES}
    ${XRANDR_LIBRARIES}
)
set(defs "")
if (${USE_ADWAITA})
    pkg_check_modules(ADWAITA REQUIRED libadwaita-1)
    set(defs ${defs} _WDS_ADWAITA)
    set(incl ${incl} ${ADWAITA_INCLUDE_DIRS})
    set(libs ${libs} ${ADWAITA_LIBRARIES})
endif()

set(CMAKE_CXX_STANDARD 20)

add_executable(${PROJECT_NAME} src/displays.cpp src/main.cpp src/ui.cpp src/wacom.cpp)
target_compile_definitions(${PROJECT_NAME} PRIVATE ${defs})
target_include_directories(${PROJECT_NAME} PRIVATE ${incl})
target_link_libraries(${PROJECT_NAME} PRIVATE ${libs})

install(TARGETS ${PROJECT_NAME} DESTINATION /usr/local/bin)
