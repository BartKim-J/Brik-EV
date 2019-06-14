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
    # Main #
    ${PRJ_INCLUDE_PATH}
    
    # System #
    ${PRJ_INCLUDE_PATH}/System

    # Socket #
    ${PRJ_INCLUDE_PATH}/Socket
        
    # Packet #
    ${PRJ_INCLUDE_PATH}/Packet/Handler
    ${PRJ_INCLUDE_PATH}/Packet
    
    # Video #
    ${PRJ_INCLUDE_PATH}/Video
    
    # Display #
    ${PRJ_INCLUDE_PATH}/Display
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

        # Socket #
        ${PRJ_MODULE_PATH}/Socket/socket_listener.c
        
        # Packet #
        ${PRJ_MODULE_PATH}/Packet/Handler/packet_cmd.c
        ${PRJ_MODULE_PATH}/Packet/Handler/packet_video.c
        ${PRJ_MODULE_PATH}/Packet/Handler/packet_audio.c
        ${PRJ_MODULE_PATH}/Packet/packet_handler.c
        ${PRJ_MODULE_PATH}/Packet/connect_mgmt.c

        # Video #
        ${PRJ_MODULE_PATH}/Video/images.c
        ${PRJ_MODULE_PATH}/Video/decoder.c
        ${PRJ_MODULE_PATH}/Video/video_handler.c
        ${PRJ_MODULE_PATH}/Video/frame_handler.c
        

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

################### Target Command ###################
# remove CMakeFiles and Build file #
add_custom_target(all_clean
    sudo rm -rf
    ${PRJ_ROOT_PATH}/CMakeFiles
    ${PRJ_ROOT_PATH}/CMakeCache.txt
    ${PRJ_ROOT_PATH}/cmake_install.cmake
    ${PRJ_ROOT_PATH}/Makefile
)

# run with debug mod(gdb) #
add_custom_target(gdb
    make all &&
    sudo sh ${PRJ_SHELL_PATH}/debug.sh
)

# run without debug mod # 
add_custom_target(run
    make all &&
    sudo sh ${PRJ_SHELL_PATH}/run.sh
)

# run forwader #
add_custom_target(airplay
    sudo ${PRJ_ROOT_PATH}/airplay_server/Airplay-ConnectivityForwarder -hqw 1920 -hqh 1080 -lqw 640 -lqh 360
)

# install #
add_custom_target(install_libc6_2_23
    sudo dpkg --install --auto-deconfigure ${PRJ_ROOT_PATH}/lib/install/libc6/libc6_2.23-0ubuntu11_armhf.deb
)

add_custom_target(install_libc6_2_27
    sudo dpkg --install --auto-deconfigure ${PRJ_ROOT_PATH}/lib/install/libc6/libc6_2.27-3ubuntu1_armhf.deb
)

add_custom_target(install_package
    sudo sh ${PRJ_ROOT_PATH}/lib/install/install_package.sh
)

# update linux config file #
add_custom_target(update_hostapd
    sudo cp ${PRJ_ROOT_PATH}/lib/config/internet/hostapd.conf /etc/hostapd/hostapd.conf
)

add_custom_target(update_dnsmasq
    sudo cp ${PRJ_ROOT_PATH}/lib/config/internet/dnsmasq.conf /etc/dnsmasq.conf
)

add_custom_target(update_udev
    sudo cp ${PRJ_ROOT_PATH}/lib/config/udev/10-local.rules /etc/udev/rules.d/10-local.rules
)



# Service #
add_custom_target(service_update
    sudo cp ${PRJ_ROOT_PATH}/lib/service/brik_network.service /etc/systemd/system/brik_network.service &&
    sudo cp ${PRJ_ROOT_PATH}/lib/service/brik_airplay.service /etc/systemd/system/brik_airplay.service &&
    sudo cp ${PRJ_ROOT_PATH}/lib/service/brik.service         /etc/systemd/system/brik.service &&
    sudo systemctl daemon-reload
)

add_custom_target(service_enable
    sudo systemctl enable brik_network.service &&
    sudo systemctl enable brik_airplay.service &&
    sudo systemctl enable brik.service 
)

add_custom_target(service_start
    sudo systemctl restart brik_airplay.service && 
    sudo systemctl restart brik.service 
)

add_custom_target(service_stop
    sudo systemctl stop brik_airplay.service &&
    sudo systemctl stop brik.service 
)

add_custom_target(service_disable
    sudo systemctl disable brik_network.service &&
    sudo systemctl disable brik_airplay.service &&
    sudo systemctl disable brik.service  
)

################### Target Setting ###################
# install #
install( DIRECTORY ${PRJ_ROOT_PATH}
    DESTINATION ${PRJ_INSTALLED_PATH}
    COMPONENT brik_install
    PATTERN .svn EXCLUDE
)

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
