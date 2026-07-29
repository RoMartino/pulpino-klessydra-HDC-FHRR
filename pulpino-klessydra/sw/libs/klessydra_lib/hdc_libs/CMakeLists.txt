set(SOURCES
    src/hdc_dispatch.cpp
    src/hdc_fhrr_hw.cpp
    src/hdc_fhrr_sw.cpp
    src/hdc_runtime.cpp
)

set(HEADERS
    inc/csr_access.hpp
    inc/hdc_class.hpp
    inc/hdc_defines.hpp
    inc/hdc_internal.hpp
    inc/hv_struct.hpp
)

add_library(hdc_libs STATIC ${SOURCES} ${HEADERS})

target_compile_options(hdc_libs PRIVATE ${KLESS_DEFINES})

target_include_directories(hdc_libs PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
)
