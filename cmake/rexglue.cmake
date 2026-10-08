# These functions attach the existing ReXGlue host adapter without duplicating
# rexcore globals or linking rexruntime back into its own object libraries.
set(XBOX_GUIDE_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/..")
get_filename_component(XBOX_GUIDE_SOURCE_DIR "${XBOX_GUIDE_SOURCE_DIR}" ABSOLUTE)

function(xbox_guide_attach_rexglue target)
    if(NOT TARGET ${target} OR NOT DEFINED REXGLUE_ROOT)
        message(FATAL_ERROR "xbox_guide_attach_rexglue requires a ReXGlue source build")
    endif()
    set(sources
        guide/dlc_catalog.cpp
        guide/guide_dlc.cpp
        guide/guide_storage.cpp
        guide/guide_input.cpp
        guide/active_downloads.cpp
        guide/file_browser.cpp
        guide/guide_list_page.cpp
        guide/message_box.cpp
        guide/guide_layout.cpp
        guide/guide_title_update.cpp
        guide/title_update.cpp
        guide/guide_notification.cpp
        guide/guide_settings.cpp
        guide/virtual_keyboard.cpp
        guide/xbox_guide.cpp
        guide/xbox_keyboard.cpp
        xui/package.cpp
        xui/renderer.cpp
        xui/runtime.cpp
        xui/resolve_file.cpp
        xui/schema.cpp
        xui/system_update.cpp
        xui/xtt_font.cpp
        xui/xur_reader.cpp)
    list(TRANSFORM sources PREPEND "${XBOX_GUIDE_SOURCE_DIR}/src/ui/")
    target_sources(${target} PRIVATE ${sources})
    target_include_directories(${target} PUBLIC
        $<BUILD_INTERFACE:${XBOX_GUIDE_SOURCE_DIR}/include>)
    target_link_libraries(${target} PRIVATE winhttp)
endfunction()

function(xbox_guide_attach_tests target)
    set(sources
        message_box_test.cpp xui_format_test.cpp xui_runtime_test.cpp xtt_font_test.cpp
        guide_font_test.cpp guide_input_test.cpp virtual_keyboard_test.cpp
        dlc_catalog_test.cpp title_update_test.cpp)
    list(TRANSFORM sources PREPEND "${XBOX_GUIDE_SOURCE_DIR}/tests/unit/ui/")
    target_sources(${target} PRIVATE ${sources})
endfunction()
