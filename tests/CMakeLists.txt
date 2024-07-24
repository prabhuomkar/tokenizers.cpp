file(GLOB TEST_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_executable(tokenizers_tests ${TEST_SOURCES})

target_link_libraries(tokenizers_tests gtest gtest_main tokenizers ICU::uc ICU::i18n)

include(GoogleTest)

gtest_discover_tests(tokenizers_tests)
