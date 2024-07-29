add_executable(run run.cpp)

target_link_libraries(run tokenizers ICU::uc ICU::i18n)
