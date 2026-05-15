cmake_minimum_required(VERSION 3.20)
project(PROI_Hotel_system_management)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(
    -Wall
    -Wextra
    -Wpedantic
    -Wconversion
    -Wsign-conversion
    -Wshadow
    -Wold-style-cast
    -Werror
)

add_library(HotelLib
    include/staff.hpp
    src/staff.cpp
    src/Hotel.cpp
    src/Simulation.cpp
)
target_include_directories(HotelLib PUBLIC include)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")
    add_executable(HotelApp src/main.cpp)
    target_link_libraries(HotelApp PRIVATE HotelLib)
endif()

include(CTest)
if(BUILD_TESTING
   AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/test_staff.cpp")
    include(FetchContent)
    FetchContent_Declare(Catch2
      GIT_REPOSITORY https://github.com/catchorg/Catch2.git
      GIT_TAG v3.8.1
    )
    FetchContent_MakeAvailable(Catch2)
    target_compile_options(Catch2 PRIVATE -Wno-error=conversion -Wno-error=sign-conversion)
    target_compile_options(Catch2WithMain PRIVATE -Wno-error=conversion -Wno-error=sign-conversion)

    add_executable(UnitTests tests/test_staff.cpp)
    target_link_libraries(UnitTests PRIVATE HotelLib Catch2::Catch2WithMain)

    add_test(NAME UnitTests COMMAND UnitTests)
endif()
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