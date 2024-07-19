add_executable(sample sample.cpp)

target_link_libraries(sample gtest gtest_main tokenizers.cpp ICU::uc ICU::i18n)
