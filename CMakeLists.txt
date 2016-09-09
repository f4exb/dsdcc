# use, i.e. don't skip the full RPATH for the build tree
set(CMAKE_SKIP_BUILD_RPATH  FALSE)

# when building, don't use the install RPATH already
# (but later on when installing)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)

SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

# add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/Modules)

project(dsdcc)

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
    dstarold.cpp
    ysf.cpp
    dpmr.cpp
    nxdn.cpp
    p25p1_heuristics.cpp
    dsd_upsample.cpp
    fec.cpp
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
    dstarold.h
    ysf.h
    dpmr.h
    nxdn.h
    p25p1_heuristics.h
    dsd_upsample.h
    runningmaxmin.h
    doublebuffer.h
    fec.h
)

include_directories(
    ${PROJECT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${LIBMBE_INCLUDE_DIR}
    ${LIBSERIALDV_INCLUDE_DIR}
)

add_library(dsdcc SHARED
    ${dsdcc_SOURCES}
)

target_link_libraries(dsdcc ${LIBMBE_LIBRARY} ${LIBSERIALDV_LIBRARY})

add_executable(dsdccx
    dsd_main.cpp
)

target_include_directories(dsdccx PUBLIC
    ${PROJECT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(dsdccx dsdcc)

install(TARGETS dsdccx DESTINATION bin)
install(TARGETS dsdcc DESTINATION lib)
install(FILES ${dsdcc_HEADERS} DESTINATION include/${PROJECT_NAME})
