cmake_minimum_required(VERSION 3.8)
project(homegear_kodi)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        src/Factory.cpp
        src/Factory.h
        src/GD.cpp
        src/GD.h
        src/KodiInterface.cpp
        src/KodiInterface.h
        src/MyCentral.cpp
        src/MyCentral.h
        src/MyFamily.cpp
        src/MyFamily.h
        src/MyPacket.cpp
        src/MyPacket.h
        src/MyPeer.cpp
        src/MyPeer.h)

add_custom_target(homegear COMMAND ../../makeAll.sh SOURCES ${SOURCE_FILES})

add_library(homegear_kodi ${SOURCE_FILES})