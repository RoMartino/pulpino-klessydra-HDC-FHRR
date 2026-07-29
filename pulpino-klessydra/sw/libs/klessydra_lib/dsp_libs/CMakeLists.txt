add_definitions(${KLESS_DEFINES})

set(SOURCES
    src/convolution.c
    src/kvcu_complete.c
    )

set(HEADERS
    inc/dsp_functions.h
    )

include_directories(inc/)

add_library(kless_dsp_libs STATIC ${SOURCES} ${HEADERS})

