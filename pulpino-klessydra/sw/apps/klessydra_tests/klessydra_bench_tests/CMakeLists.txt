# hack to make sure the perfbench.* libraries compile
include_directories(
  ${CMAKE_SOURCE_DIR}/libs/bench_lib/inc
  ${CMAKE_SOURCE_DIR}/libs/stdio_lib/inc
  ${CMAKE_SOURCE_DIR}/libs/string_lib/inc
  ${CMAKE_SOURCE_DIR}/libs/sys_lib/inc
  .
)

set(PERFBENCH_CORE_SOURCES main.c)

add_library(perfbench.core2 STATIC ${PERFBENCH_CORE_SOURCES})

add_subdirectory(fft)
add_subdirectory(CoreMark)
add_subdirectory(cfd)
