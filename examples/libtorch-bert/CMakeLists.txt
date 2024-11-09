set(CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/libtorch")
find_package(Torch REQUIRED)

add_executable(libtorch-bert mobilebert-uncased.cpp)

target_link_libraries(libtorch-bert tokenizers ${TORCH_LIBRARIES})
