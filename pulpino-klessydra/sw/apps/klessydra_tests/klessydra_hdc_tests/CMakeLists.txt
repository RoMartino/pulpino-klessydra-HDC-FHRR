include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/common
)

add_subdirectory(fhrr_bind_test)
add_subdirectory(fhrr_bundle_test)
add_subdirectory(fhrr_similarity_test)
add_subdirectory(fhrr_clip_test)
add_subdirectory(fhrr_encode_test)
add_subdirectory(fhrr_permute_test)
add_subdirectory(fhrr_ucihar_inference)
