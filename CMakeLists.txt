cmake_minimum_required(VERSION 3.26)

project(cannonhill LANGUAGES CXX VERSION 1.0.0)
add_subdirectory(deps)
set(CMAKE_CXX_STANDARD 20)

add_executable(cannonhill menue.cpp panzer.cpp panzer.h audio.cpp audio.hpp mixer.cpp mixer.hpp)

if (WIN32 OR WIN64)
   configure_file(${CMAKE_SOURCE_DIR}/deps/cannonhill.rc.in ${PROJECT_BINARY_DIR}/cannonhill.rc)
   target_sources(cannonhill PRIVATE "${PROJECT_BINARY_DIR}/cannonhill.rc")

    # Copy dependencies
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${SDL2_DLLS}
            $<TARGET_FILE_DIR:${PROJECT_NAME}>)

    install(FILES ${SDL2_DLLS} DESTINATION .
            CONFIGURATIONS RelWithDebInfo Debug Release
    )
    target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE
            ${SDL2_LIBRARIES}
    )
    target_include_directories(
            ${CMAKE_PROJECT_NAME}
            PRIVATE
            ${SDL2_INCLUDE_DIRS}
    )
    # enable incremental linking
    set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES LINK_FLAGS "/INCREMENTAL:YES")
    # enable edit and continue
    set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES LINK_FLAGS "/EDITANDCONTINUE")
    
    # set platform to application instead of console
    set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS")
else()
    find_package(SDL2 REQUIRED)
    target_link_libraries(cannonhill SDL2::SDL2)
endif()

install(TARGETS cannonhill CONFIGURATIONS RelWithDebInfo Debug Release DESTINATION .)
install(DIRECTORY res/sounds DESTINATION . CONFIGURATIONS RelWithDebInfo Debug Release)
install(DIRECTORY res/img DESTINATION . CONFIGURATIONS RelWithDebInfo Debug Release)
