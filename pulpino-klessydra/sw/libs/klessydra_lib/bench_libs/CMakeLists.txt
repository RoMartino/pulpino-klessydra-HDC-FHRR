set(SOURCES
    src/core_portme.c
    )

add_definitions(${COREMARK_DEFS})


set(HEADERS
    inc/coremark.h
    inc/core_portme.h
    )

include_directories(inc/)

add_library(kless_bench_libs STATIC ${SOURCES} ${HEADERS})

