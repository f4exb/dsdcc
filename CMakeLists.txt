list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/Modules)

project(dsdcc)

find_package(LibMbe REQUIRED)

set(dsdcc_SOURCES
    descramble.cpp
    dmr_data.cpp
    dmr_voice.cpp
    dsd_decoder.cpp
    dsd_filters.cpp
    dsd_mbe.cpp
    dsd_opts.cpp
    dsd_state.cpp
    dstar.cpp
)

set(dsdcc_HEADERS
    descramble.h
    dmr_data.h
    dmr_voice.h
    dsd_decoder.h
    dsd_filters.h
    dsd_mbe.h
    dsd_opts.h
    dsd_state.h
    dstar.h
)

include_directories(
    ${PROJECT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${LIBMBE_INCLUDE_DIR}
)

add_library(dsdcc SHARED
    ${dsdcc_SOURCES}
)

target_link_libraries(dsdcc ${LIBMBE_LIBRARY})

install(TARGETS dsdcc DESTINATION lib)
