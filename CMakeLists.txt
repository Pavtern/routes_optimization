cmake_minimum_required(VERSION 3.28)
project(CMakeSFMLProject LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

include(FetchContent)
FetchContent_Declare(SFML
        GIT_REPOSITORY https://github.com/SFML/SFML.git
        GIT_TAG 2.6.x
        GIT_SHALLOW ON
        EXCLUDE_FROM_ALL
        SYSTEM)
FetchContent_MakeAvailable(SFML)

add_executable(routes_optimization main.cpp
)
target_link_libraries(routes_optimization PRIVATE sfml-graphics)
target_compile_features(routes_optimization PRIVATE cxx_std_23)

if(MSVC)
    target_compile_options(routes_optimization PRIVATE /W4 /WX)
else()
    target_compile_options(routes_optimization PRIVATE -Wall -Wextra -Wpedantic -Werror  -Wno-missing-field-initializers)
endif()

if(WIN32)
    add_custom_command(
            TARGET routes_optimization
            COMMENT "Copy OpenAL DLL"
            PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${SFML_SOURCE_DIR}/extlibs/bin/$<IF:$<EQUAL:${CMAKE_SIZEOF_VOID_P},8>,x64,x86>/openal32.dll $<TARGET_FILE_DIR:routes_optimization>
            VERBATIM)
endif()