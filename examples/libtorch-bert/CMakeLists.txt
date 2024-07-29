set(CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/libtorch")
find_package(Torch REQUIRED)

link_directories("${CMAKE_BINARY_DIR}")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")

add_executable(libtorch-bert bert-base-uncased.cpp)

target_link_libraries(libtorch-bert tokenizers ICU::uc ICU::i18n)
target_link_libraries(libtorch-bert ${TORCH_LIBRARIES})
