add_executable(sample sample.cpp)
add_executable(run run.cpp)

target_link_libraries(sample tokenizers ICU::uc ICU::i18n)
target_link_libraries(run tokenizers ICU::uc ICU::i18n)
