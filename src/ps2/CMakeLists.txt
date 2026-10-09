#  ____     ___ |    / _____ _____
# |  __    |    |___/    |     |
# |___| ___|    |    \ __|__   |     gsKit Open Source Project.
# ----------------------------------------------------------------------
# Copyright 2004 - Chris "Neovanglist" Gilbert <Neovanglist@LainOS.org>
# Licenced under Academic Free License version 2.0
# Review gsKit README & LICENSE files for further details.

cmake_minimum_required(VERSION 3.21)

project(ps2uta)

if(NOT DEFINED ENV{PS2DEV})
	message(FATAL_ERROR "Please set $PS2DEV environment variable")
endif()
set(PS2DEV $ENV{PS2DEV})

if(NOT DEFINED ENV{GSKIT})
	set(GSKIT ${PS2DEV}/gsKit)
else()
	set(GSKIT $ENV{GSKIT})
endif()

set(GSKIT_EXTERNAL_LIBS "")
set(GSKIT_EXTERNAL_INCLUDES "")

find_package(PkgConfig REQUIRED)

pkg_check_modules(LIBJPEG libjpeg)
if(LIBJPEG_FOUND)
	list(APPEND GSKIT_EXTERNAL_LIBS ${LIBJPEG_LIBRARIES})
	list(APPEND GSKIT_EXTERNAL_INCLUDES ${LIBJPEG_INCLUDE_DIRS})
	add_definitions(-DHAVE_LIBJPEG)
endif()

pkg_check_modules(LIBTIFF libtiff-4)
if(LIBTIFF_FOUND)
	list(APPEND GSKIT_EXTERNAL_LIBS ${LIBTIFF_LIBRARIES})
	list(APPEND GSKIT_EXTERNAL_INCLUDES ${LIBTIFF_INCLUDE_DIRS})
	add_definitions(-DHAVE_LIBTIFF)
endif()

pkg_check_modules(LIBPNG libpng)
if(LIBPNG_FOUND)
	list(APPEND GSKIT_EXTERNAL_LIBS ${LIBPNG_LIBRARIES})
	list(APPEND GSKIT_EXTERNAL_INCLUDES ${LIBPNG_INCLUDE_DIRS})
	add_definitions(-DHAVE_LIBPNG)
endif()

include_directories(${GSKIT_EXTERNAL_INCLUDES})
# include_directories(third_party/libschrift/)
include_directories(../)
include_directories(../../)

set(CMAKE_EXECUTABLE_SUFFIX .elf)

add_executable(uta
	../backend.c
	../coregame2.c
	../../gen/coregame.c
	../../gen/ext1.c
	ps2frontend.c
)
target_link_libraries(uta
	gskit_toolkit gskit dmakit pad ${GSKIT_EXTERNAL_LIBS}
)
