cmake_minimum_required(VERSION 3.15)
project(search_engine)

set(CMAKE_CXX_STANDARD 20)

include_directories(include resources src)
# Подключение библиотеки nlohmann/json
include(FetchContent)
set(FETCHCONTENT_TRY_FIND_PACKAGE_MODE NEVER)
FetchContent_Declare(
        json
        URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(json)

# Подключение Google Test
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
if(MSVC)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
endif()
set(gtest_disable_pthreads ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

enable_testing()

# Библиотека с общим кодом
add_library(search_engine_lib
        src/ConverterJSON.cpp
        src/InvertedIndex.cpp
        src/SearchServer.cpp
)

target_include_directories(search_engine_lib PUBLIC include)
target_link_libraries(search_engine_lib PUBLIC nlohmann_json::nlohmann_json)

# Исполняемый файл с тестами
add_executable(search_engine_tests tests/main.cpp)
target_include_directories(search_engine_tests PRIVATE include)
target_link_libraries(search_engine_tests PRIVATE search_engine_lib gtest_main)

# Основное приложение
add_executable(search_app src/search_app.cpp)
target_include_directories(search_app PRIVATE include)
target_link_libraries(search_app PRIVATE search_engine_lib)

include(GoogleTest)
gtest_discover_tests(search_engine_tests)
