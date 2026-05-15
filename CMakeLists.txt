cmake_minimum_required(VERSION 3.20)
project(PROI_zad4)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
    add_compile_options(/W4 /WX)
else ()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif ()

include_directories(include)

add_library(hotel_logic
        src/Apartment.cpp
        src/DoubleRoom.cpp
        src/Room.cpp
        src/SingleRoom.cpp
        src/TripleRoom.cpp
)

add_executable(PROI_zad4 app/main.cpp)
target_link_libraries(PROI_zad4 hotel_logic)

add_executable(PROI_tests app/tests.cpp)
target_link_libraries(PROI_tests hotel_logic)