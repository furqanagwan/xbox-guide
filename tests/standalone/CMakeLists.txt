add_executable(xbox_guide_core_tests core_test.cpp)
target_link_libraries(xbox_guide_core_tests PRIVATE xbox_guide::core Catch2::Catch2WithMain)
target_include_directories(xbox_guide_core_tests PRIVATE ../unit/ui)
add_test(NAME xbox_guide.core COMMAND xbox_guide_core_tests)
