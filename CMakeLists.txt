cmake_minimum_required(VERSION 3.26)

project(cannonhill LANGUAGES CXX)
add_subdirectory(deps)
set(CMAKE_CXX_STANDARD 17)

add_executable(cannonhill menue.cpp panzer.cpp panzer.h audio.cpp audio.hpp mixer.cpp mixer.hpp)

if (WIN32 OR WIN64)
    if (MSVC)
        add_definitions(/MP /d2FH4-)
    endif()

    # Copy dependencies
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${SDL2_DLLS}
            $<TARGET_FILE_DIR:${PROJECT_NAME}>)

    install(FILES ${SDL2_DLLS} DESTINATION
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
else()
    find_package(SDL2 REQUIRED)
    target_link_libraries(cannonhill SDL2::SDL2)
endif()
