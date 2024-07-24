add_executable(sample sample.cpp)

target_link_libraries(sample gtest gtest_main tokenizers ICU::uc ICU::i18n)
