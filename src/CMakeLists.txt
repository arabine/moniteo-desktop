cmake_minimum_required(VERSION 3.5)

set(MONITEO_PROJECT moniteo)

project(${MONITEO_PROJECT} LANGUAGES C CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set (CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# set(CMAKE_VERBOSE_MAKEFILE on)


if(POLICY CMP0072)
    cmake_policy(SET CMP0072 NEW)
endif()
find_package(OpenGL REQUIRED)


if (UNIX AND NOT APPLE)
    add_definitions(-DUSE_UNIX_OS -DUSE_LINUX_OS)
    add_link_options(-static-libgcc -static-libstdc++)
endif()

if(WIN32)
    if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        add_compile_options(/FS /bigobj)
    endif()
    add_definitions(-DUSE_WINDOWS_OS)
endif()

add_definitions(-DUNICODE)

set(SRCS
    main.cpp
    WindowBase.h
    ConsoleWindow.cpp
    ConsoleWindow.h

    Util.cpp
    Log.cpp

    MainWindow.cpp
    MainWindow.h

    CourseWindow.cpp
    CourseWindow.h

    TableWindow.cpp
    TableWindow.h

    Gui.h
    Gui.cpp

    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/backends/imgui_impl_glfw.cpp
    imgui/backends/imgui_impl_opengl2.cpp
    ImGuiColorTextEdit/TextEditor.cpp
    ImGuiColorTextEdit/TextEditor.h
    ImGuiFileDialog/ImGuiFileDialog.cpp

    zebrafx7500/Zebra7500.cpp
  #  zebrafx7500/Zebra7500Util.cpp
    
)

if(WIN32)
    list(APPEND
        SRCS
        icon.rc
    )
endif()

add_executable(${MONITEO_PROJECT}
    ${SRCS}
)

add_definitions(-DIMGUI_IMPL_OPENGL_LOADER_GL3W)
add_definitions(-Dcimg_display=0)


if (UNIX)
target_link_libraries(${MONITEO_PROJECT}
    manolab
    icl
    dl
    SDL2
    pthread
    udev
    OpenGL::GL
    ssl crypto
)
elseif(WIN32)


target_link_libraries(${MONITEO_PROJECT}
    OpenGL::GL
    glfw3
    rfidapi32pc comctl32
    ws2_32.lib psapi.lib setupapi.lib cfgmgr32.lib advapi32.lib Dbghelp.lib 
)

target_compile_definitions(${MONITEO_PROJECT} PUBLIC _CRT_SECURE_NO_WARNINGS _CRT_NON_CONFORMING_SWPRINTFS _WINSOCK_DEPRECATED_NO_WARNINGS)


if( MSVC )
    SET( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ENTRY:mainCRTStartup /SUBSYSTEM:windows /NODEFAULTLIB:library" )
endif()

target_link_directories(${MONITEO_PROJECT} PUBLIC ${OPENSSL_DIR}/lib glfw/lib-vc2019 zebrafx7500/libs/win64)

endif()

target_include_directories(
    ${MONITEO_PROJECT}
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/gl3w
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends
    ${CMAKE_CURRENT_SOURCE_DIR}/ImGuiColorTextEdit
    ${CMAKE_CURRENT_SOURCE_DIR}/ImGuiFileDialog
    ${OPENSSL_DIR}/include
    glfw/include
    ${CMAKE_CURRENT_SOURCE_DIR}/zebrafx7500
)
