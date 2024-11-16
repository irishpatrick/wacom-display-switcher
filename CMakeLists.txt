cmake_minimum_required(VERSION 3.18)

project(wacomswitcher)

option(USE_ADWAITA "use adwaita" OFF)
set(WINDOW_SYSTEM "x11" CACHE STRING "x11 or wayland")
set(APP_VERSION "noversion" CACHE STRING "app version")

if (NOT ${WINDOW_SYSTEM} STREQUAL "x11" AND NOT ${WINDOW_SYSTEM} STREQUAL "wayland")
    message(FATAL_ERROR "unknown window system: ${WINDOW_SYSTEM}")
endif ()

set(CMAKE_CXX_STANDARD 20)

set(USE_X11 0)
set(USE_WAYLAND 0)
if (${WINDOW_SYSTEM} STREQUAL "x11")
    set(USE_X11 1)
endif ()
if (${WINDOW_SYSTEM} STREQUAL "wayland")
    set(USE_WAYLAND 1)
endif ()

include(FindPkgConfig)
pkg_check_modules(GTK4 REQUIRED gtk4)
if (${USE_X11})
    pkg_check_modules(XRANDR REQUIRED xrandr)
    pkg_check_modules(XLIB REQUIRED x11)
    pkg_check_modules(XINPUT REQUIRED xi)
endif ()
if (${USE_WAYLAND})
    pkg_check_modules(LIBINPUT REQUIRED libinput)
endif ()
pkg_check_modules(UDEV REQUIRED libudev)

set(incl
        ${GTK4_INCLUDE_DIRS}
        ${UDEV_INCLUDE_DIRS}
)
if (${USE_X11})
    set(incl
            ${incl}
            ${XINPUT_INCLUDE_DIRS}
            ${XLIB_INCLUDE_DIRS}
            ${XRANDR_INCLUDE_DIRS}
    )
endif ()

set(libs
        ${GTK4_LIBRARIES}
        ${UDEV_LIBRARIES}
)
if (${USE_X11})
    set(libs
            ${libs}
            ${XINPUT_LIBRARIES}
            ${XLIB_LIBRARIES}
            ${XRANDR_LIBRARIES}
    )
endif ()

set(defs _APP_VERSION="${APP_VERSION}")
if (${USE_ADWAITA})
    pkg_check_modules(ADWAITA REQUIRED libadwaita-1)
    set(defs ${defs} _WDS_ADWAITA)
    set(incl ${incl} ${ADWAITA_INCLUDE_DIRS})
    set(libs ${libs} ${ADWAITA_LIBRARIES})
endif ()

if (${USE_X11})
    set(defs ${defs} _WDS_X11)
endif ()
if (${USE_WAYLAND})
    set(defs ${defs} _WDS_WAYLAND)
endif ()


add_executable(${PROJECT_NAME}
        src/displays.cpp
        src/instancemutex.cpp
        src/main.cpp
        src/ui.cpp
        src/wacom.cpp
)
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Werror)
target_compile_definitions(${PROJECT_NAME} PRIVATE ${defs})
target_include_directories(${PROJECT_NAME} PRIVATE ${incl})
target_link_libraries(${PROJECT_NAME} PRIVATE ${libs})

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
