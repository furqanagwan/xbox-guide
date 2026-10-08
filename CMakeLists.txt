cmake_minimum_required(VERSION 3.25)
project(XboxGuide VERSION 0.1.0 LANGUAGES CXX)

if(NOT WIN32)
    message(FATAL_ERROR "Xbox Guide currently supports Windows hosts only")
endif()

option(XBOX_GUIDE_BUILD_CORE "Build the reusable scene library" ${PROJECT_IS_TOP_LEVEL})
option(XBOX_GUIDE_BUILD_TESTS "Build standalone synthetic tests" ${PROJECT_IS_TOP_LEVEL})
option(XBOX_GUIDE_FETCH_DEPENDENCIES "Fetch pinned fmt, ImGui and Catch2 when not supplied" OFF)

if(XBOX_GUIDE_BUILD_CORE)
    include(cmake/dependencies.cmake)
    add_library(xbox_guide_core STATIC
        src/ui/xui/package.cpp
        src/ui/xui/schema.cpp
        src/ui/xui/xur_reader.cpp
        src/ui/xui/runtime.cpp
        src/ui/xui/renderer.cpp
        src/ui/guide/active_downloads.cpp
        src/ui/guide/file_browser.cpp
        src/ui/guide/guide_list_page.cpp
        src/ui/guide/message_box.cpp
        src/ui/guide/guide_layout.cpp
        src/ui/guide/virtual_keyboard.cpp
    )
    add_library(xbox_guide::core ALIAS xbox_guide_core)
    target_compile_features(xbox_guide_core PUBLIC cxx_std_23)
    target_include_directories(xbox_guide_core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_link_libraries(xbox_guide_core PUBLIC fmt::fmt imgui::imgui)
    if(XBOX_GUIDE_BUILD_TESTS)
        enable_testing()
        add_subdirectory(tests/standalone)
    endif()
elseif(XBOX_GUIDE_BUILD_TESTS)
    message(FATAL_ERROR "Standalone tests require XBOX_GUIDE_BUILD_CORE=ON")
endif()

include(cmake/rexglue.cmake)
