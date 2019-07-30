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
        src/KodiCentral.cpp
        src/KodiCentral.h
        src/Kodi.cpp
        src/Kodi.h
        src/KodiPacket.cpp
        src/KodiPacket.h
        src/KodiPeer.cpp
        src/KodiPeer.h)

add_custom_target(homegear COMMAND ../../makeAll.sh SOURCES ${SOURCE_FILES})

add_library(homegear_kodi ${SOURCE_FILES})