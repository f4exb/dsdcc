cmake_minimum_required(VERSION 3.1.0)

# use c++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# use, i.e. don't skip the full RPATH for the build tree
set(CMAKE_SKIP_BUILD_RPATH  FALSE)

# when building, don't use the install RPATH already
# (but later on when installing)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)

SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

# add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# Compiler flags.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -fmax-errors=10 -O2 -ffast-math -ftree-vectorize ${EXTRA_FLAGS}")

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/Modules)

project(dsdcc)

set(MAJOR_VERSION 1)
set(MINOR_VERSION 8)
set(PATCH_VERSION 5)
set(PACKAGE libdsdcc)
set(VERSION_STRING ${MAJOR_VERSION}.${MINOR_VERSION}.${PATCH_VERSION})
set(VERSION ${VERSION_STRING})

include(GNUInstallDirs)
set(LIB_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}") # "lib" or "lib64"

if (BUILD_TYPE MATCHES RELEASE)
    set(CMAKE_BUILD_TYPE "Release")
elseif (BUILD_TYPE MATCHES RELEASEWITHDBGINFO)
    set(CMAKE_BUILD_TYPE "ReleaseWithDebugInfo")
elseif (BUILD_TYPE MATCHES DEBUG)
    set(CMAKE_BUILD_TYPE "Debug")
elseif (BUILD_TYPE MATCHES DEBIAN)
    set(CMAKE_BUILD_TYPE "Release")
    set(BUILD_DEBIAN TRUE)
else()
    set(CMAKE_BUILD_TYPE "Release")
endif()

find_package(LibMbe)
find_package(SerialDV)

if (USE_MBELIB AND LIBMBE_FOUND)
    add_definitions(-DDSD_USE_MBELIB)
endif()

if (LIBSERIALDV_FOUND)
    add_definitions(-DDSD_USE_SERIALDV)
endif()

set(dsdcc_SOURCES
    descramble.cpp
    dmr.cpp
    dsd_decoder.cpp
    dsd_filters.cpp
    dsd_logger.cpp
    dsd_mbe.cpp
    dsd_opts.cpp
    dsd_state.cpp
    dsd_symbol.cpp
    dstar.cpp
    ysf.cpp
    dpmr.cpp
    nxdn.cpp
    nxdnconvolution.cpp
    nxdncrc.cpp
    nxdnmessage.cpp
    p25p1_heuristics.cpp
    dsd_upsample.cpp
    fec.cpp
    viterbi.cpp
    viterbi3.cpp
    viterbi5.cpp
    crc.cpp
    pn.cpp
    mbefec.cpp
    locator.cpp
    phaselock.cpp
    timeutil.cpp
)

set(dsdcc_HEADERS
    descramble.h
    dmr.h
    dsd_decoder.h
    dsd_filters.h
    dsd_logger.h
    dsd_mbe.h
    dsd_opts.h
    dsd_state.h
    dsd_symbol.h
    dstar.h
    ysf.h
    dpmr.h
    nxdn.h
    nxdnconvolution.h
    nxdncrc.h
    nxdnmessage.h
    p25p1_heuristics.h
    dsd_upsample.h
    runningmaxmin.h
    doublebuffer.h
    fec.h
    viterbi.h
    viterbi3.h
    viterbi5.h
    crc.h
    pn.h
    mbefec.h
    locator.h
    phaselock.h
    iirfilter.h
    timeutil.h
    export.h
)

if (USE_MBELIB AND LIBMBE_FOUND)
    set(dsdcc_HEADERS
        ${dsdcc_HEADERS}
        dsd_mbelib.h
    )
endif()

include_directories(
    ${PROJECT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

if (USE_MBELIB AND LIBMBE_FOUND)
    include_directories(
        ${LIBMBE_INCLUDE_DIR}
    )
endif()

if (LIBSERIALDV_FOUND)
    include_directories(
        ${LIBSERIALDV_INCLUDE_DIR}
    )
endif()

add_library(dsdcc SHARED
    ${dsdcc_SOURCES}
)

if (USE_MBELIB AND LIBMBE_FOUND)
    target_link_libraries(dsdcc ${LIBMBE_LIBRARY})
endif()

if (LIBSERIALDV_FOUND)
    target_link_libraries(dsdcc ${LIBSERIALDV_LIBRARY})
endif()

if (BUILD_DEBIAN)
    add_subdirectory(mbelib)
    add_subdirectory(serialdv)
    add_definitions(-DDSD_USE_SERIALDV)
    include_directories(${LIBSERIALDVSRC})
    target_link_libraries(dsdcc serialdv)
    add_definitions(-DDSD_USE_MBELIB)
    include_directories(${LIBMBELIBSRC})
    target_link_libraries(dsdcc mbelib)
endif()

add_executable(dsdccx
    dsd_main.cpp
)

set_target_properties(dsdccx PROPERTIES VERSION ${VERSION} SOVERSION ${MAJOR_VERSION})

target_include_directories(dsdccx PUBLIC
    ${PROJECT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(dsdccx dsdcc)

########################################################################
# Create Pkg Config File
########################################################################

# use space-separation format for the pc file
STRING(REPLACE ";" " " DSDCC_PC_REQUIRES "${DSDCC_PC_REQUIRES}")
STRING(REPLACE ";" " " DSDCC_PC_CFLAGS "${DSDCC_PC_CFLAGS}")
STRING(REPLACE ";" " " DSDCC_PC_LIBS "${DSDCC_PC_LIBS}")

# unset these vars to avoid hard-coded paths to cross environment
IF(CMAKE_CROSSCOMPILING)
    UNSET(DSDCC_PC_CFLAGS)
    UNSET(DSDCC_PC_LIBS)
ENDIF(CMAKE_CROSSCOMPILING)

CONFIGURE_FILE(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/libdsdcc.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/cmake/libdsdcc.pc
@ONLY)

INSTALL(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/cmake/libdsdcc.pc
    DESTINATION ${LIB_INSTALL_DIR}/pkgconfig
)

# Installation
install(TARGETS dsdccx DESTINATION bin)
install(TARGETS dsdcc DESTINATION lib${LIB_SUFFIX})
install(FILES ${dsdcc_HEADERS} DESTINATION include/${PROJECT_NAME})
