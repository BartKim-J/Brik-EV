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

# Output Paths #
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PRJ_BINARY_PATH}/build)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PRJ_BINARY_PATH}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PRJ_BINARY_PATH}/lib)

# cmake #
# cmake module #

################### Library & Source & Include Setting ###################
# link library (-l)
link_libraries(pthread avcodec avfilter avformat avutil swscale SDL m)

# link library directory (-L)
link_directories(/usr/local/lib /usr/lib)

# Include - Local #
include_directories(
    ${PRJ_INCLUDE_PATH}
    ${PRJ_INCLUDE_PATH}/sys
)

# Source #
set(
        # Sources Name #
        Source

        # Main #
        ${PRJ_SOURCE_PATH}/brik.c

        # System #
        ${PRJ_SOURCE_PATH}/brik_error.c
        ${PRJ_SOURCE_PATH}/sys/sps_parser.c
        ${PRJ_SOURCE_PATH}/sys/threadqueue.c

        # Packet #
        ${PRJ_SOURCE_PATH}/packet_handler.c
        ${PRJ_SOURCE_PATH}/connect_mgmt.c

        # Video #
        ${PRJ_SOURCE_PATH}/video_handler.c

        # Display #
#       ${PRJ_SOURCE_PATH}/display_handler.c
        ${PRJ_SOURCE_PATH}/display_handler_sdl.c
#       ${PRJ_SOURCE_PATH}/display_handler_sdl2.c
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

# Mode #
set(
    # Mode Name #
    Mode

    # Debug #
    -DDEBUG_MODE=true

    # Demo #
    -DDEMO_MODE=false

    # Release #
    -DRELEASE_MODE=false
)

# Config #
set(
    # Config Name #
    Config

    # Mode #
    ${Mode}
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
    /home/brik/brik_ev_c/airplay_server/bin/Airplay-ConnectivityForwarder -hqw 1920 -hqh 1080 -lqw 1280 -lqh 720
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
