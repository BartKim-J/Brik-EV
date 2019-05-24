#
# ============================================================================
# Name        : BrikEVC
# Author      : bato
# ============================================================================
#
######################## Base Info Setting ########################
# Header #
cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_EXTENSIONS FALSE)

# Project Info  #
project("BrikEVC")

set(VERSION_MAJOR 0)
set(VERSION_MINOR 1)

set(CMAKE_C_COMPILER gcc)

#set(CMAKE_BUILD_TYPE Debug)
#set(CMAKE_VERBOSE_MAKEFILE false)

set(PRJ_BINARY_NAME BrikEVC_exe)

# Paths #
set(PRJ_INSTALLED_PATH /Brik/Brik_ev_c)

set(PRJ_ROOT_PATH .)
set(PRJ_LIB_PATH   ${PRJ_ROOT_PATH}/lib)
set(PRJ_SHELL_PATH ${PRJ_LIB_PATH}/shell)

set(PRJ_INCLUDE_PATH ${PRJ_ROOT_PATH}/include)
set(PRJ_SOURCE_PATH	${PRJ_ROOT_PATH}/src)
set(PRJ_BINARY_PATH ${PRJ_ROOT_PATH}/bin)

set(PRJ_MODULE_PATH ${PRJ_SOURCE_PATH}/Module)
set(PRJ_SYSETM_PATH ${PRJ_SOURCE_PATH}/System)

# Output Paths #
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PRJ_BINARY_PATH}/build)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PRJ_BINARY_PATH}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PRJ_BINARY_PATH}/lib)

# cmake #
# cmake module #

################### Library & Source & Include Setting ###################
# link library (-l)
link_libraries(pthread avcodec avfilter avformat avutil swscale SDL SDL_image m)

# link library directory (-L)
link_directories(/usr/local/lib /usr/lib)

# Include - Local #
include_directories(
    ${PRJ_INCLUDE_PATH}
    ${PRJ_INCLUDE_PATH}/System
    ${PRJ_INCLUDE_PATH}/Packet
    ${PRJ_INCLUDE_PATH}/Display
    ${PRJ_INCLUDE_PATH}/Video
)

# Source #
set(
        # Sources Name #
        Source

        # Main #
        ${PRJ_SOURCE_PATH}/brik.c

        # System #
        ${PRJ_SYSETM_PATH}/error_handler.c
        ${PRJ_SYSETM_PATH}/sps_parser.c
        ${PRJ_SYSETM_PATH}/threadqueue.c

        # Packet #
        ${PRJ_MODULE_PATH}/Packet/packet_handler.c
        ${PRJ_MODULE_PATH}/Packet/connect_mgmt.c

        # Video #
        ${PRJ_MODULE_PATH}/Video/images.c
        ${PRJ_MODULE_PATH}/Video/decoder.c
        ${PRJ_MODULE_PATH}/Video/video_handler.c

        # Display #
        ${PRJ_MODULE_PATH}/Display/display.c
        ${PRJ_MODULE_PATH}/Display/display_sdl.c
        ${PRJ_MODULE_PATH}/Display/display_sdl2.c
)

# Flags #
set(
    CMAKE_SHARED_LINKER_FLAGS
    
    # Map #
     -Xlinker -map=output.map
)

set(
    # Flags Name #
    CMAKE_CXX_FLAGS

    # GDB #
    -g

    # Warning #
    -Wall
    
    ${CMAKE_SHARED_LINKER_FLAGS}
)

# "Debug" Link option
set(CMAKE_C_FLAGS_DEBUG "-DDEBUG -DC_FLAGS")
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "-DDEBUG -DLINKER_FLAGS")

# "Release" Link option
set(CMAKE_C_FLAGS_RELEASE "-DRELEASE -DC_FLAGS")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-DRELEASE -DLINKER_FLAGS")

# Config #
set(
    # Config Name #
    Config

    -DDISPLAY_TYPE_SDL
    #-DDISPLAY_TYPE_SDL2    
    #-DDISPLAY_TYPE_FB

)

# Command #

add_custom_target(all_clean
    sudo rm -rf
    ${PRJ_ROOT_PATH}/CMakeFiles
    ${PRJ_ROOT_PATH}/CMakeCache.txt
    ${PRJ_ROOT_PATH}/cmake_install.cmake
    ${PRJ_ROOT_PATH}/Makefile
)

add_custom_target(gdb
    make all &&
    sudo sh ${PRJ_SHELL_PATH}/debug.sh
)

add_custom_target(run
    make all &&
    sudo sh ${PRJ_SHELL_PATH}/run.sh
)

add_custom_target(airplay
    sudo LD_LIBRARY_PATH=.:/home/brik/brik_ev_c/airplay_server/bin/lib/ 
    /home/brik/brik_ev_c/airplay_server/bin/Airplay-ConnectivityForwarder -hqw 1920 -hqh 1080 -lqw 640 -lqh 360
)

# install #
install( DIRECTORY ${PRJ_ROOT_PATH}
    DESTINATION ${PRJ_INSTALLED_PATH}
    COMPONENT brik_install
    PATTERN .svn EXCLUDE
)

################### Target Setting ###################
# Install exe file #
add_executable(${PRJ_BINARY_NAME} ${Source})

# Compile Option  #
target_compile_options(${PRJ_BINARY_NAME} PRIVATE ${CMAKE_CXX_FLAGS})

# Compile Config #
target_compile_definitions(${PRJ_BINARY_NAME} PRIVATE ${Config} BOOST_ERROR_CODE_HEADER_ONLY)

################### CMakeLists.txt Done ###################

################### Messgae ###################
message("###########################################")
message("#")
message("# Project Name   :: Brik Project.")
message("# exe File       :: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PRJ_BINARY_NAME}")
message("# Version        :: ${VERSION_MAJOR}.${VERSION_MINOR}v")
message("# Build Option   :: C ${CMAKE_CXX_STANDARD} STANDARD ")
message("# Flags          :: ${CMAKE_CXX_FLAGS}")
message("# Configs        :: ${Config}")
message("#")
message("###########################################")
