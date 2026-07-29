include_directories(
  ${CMAKE_SOURCE_DIR}/libs/bench_lib/inc
  ${CMAKE_SOURCE_DIR}/libs/stdio_lib/inc
  ${CMAKE_SOURCE_DIR}/libs/string_lib/inc
  ${CMAKE_SOURCE_DIR}/libs/sys_lib/inc
  .
)

add_subdirectory(convolution_test)
add_subdirectory(vcu_instruction_test)
add_subdirectory(fft_test)
add_subdirectory(Kmeans)
add_subdirectory(Matmul)
add_subdirectory(composite_test)
add_subdirectory(misc)
add_subdirectory(VGG-16)