cmake_minimum_required(VERSION 3.20)
project(PROI_Hotel_system_management)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (MSVC)
    add_compile_options(/W4 /WX)
else ()
    add_compile_options(
            -Wall -Wextra -Wpedantic -Werror
            -Wconversion -Wsign-conversion -Wshadow -Wold-style-cast
    )
endif ()

add_library(hotel_logic
        src/staff.cpp
        src/Hotel.cpp
        src/Simulation.cpp
        src/Apartment.cpp
        src/DoubleRoom.cpp
        src/Room.cpp
        src/SingleRoom.cpp
        src/TripleRoom.cpp
        src/PowerOutage.cpp
        include/PowerOutage.h
        src/FireAlarm.cpp
        include/FireAlarm.h
        src/EventGenerator.cpp
        include/EventGenerator.h
        src/HotelEvent.cpp
        include/HotelEvent.h
        src/Reservation.cpp
        include/Reservation.h
        src/Bill.cpp
        include/Bill.h
)

target_include_directories(hotel_logic PUBLIC include)
target_compile_options(hotel_logic PRIVATE -Wno-error)

if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")
    add_executable(HotelApp src/main.cpp)
    target_link_libraries(HotelApp PRIVATE hotel_logic)
endif ()

if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/app/main.cpp")
    add_executable(PROI_zad4 app/main.cpp)
    target_link_libraries(PROI_zad4 hotel_logic)
endif ()

include(CTest)
if (BUILD_TESTING)
    include(FetchContent)
    FetchContent_Declare(Catch2
            GIT_REPOSITORY https://github.com/catchorg/Catch2.git
            GIT_TAG v3.8.1
    )
    FetchContent_MakeAvailable(Catch2)
    target_compile_options(Catch2 PRIVATE -Wno-error)
    target_compile_options(Catch2WithMain PRIVATE -Wno-error)

    if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/test_staff.cpp")
        add_executable(UnitTests tests/test_staff.cpp)
        target_link_libraries(UnitTests PRIVATE hotel_logic Catch2::Catch2WithMain)
        add_test(NAME UnitTests COMMAND UnitTests)
    endif ()

    if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/app/tests.cpp")
        add_executable(PROI_tests app/tests.cpp)
        target_link_libraries(PROI_tests hotel_logic Catch2::Catch2WithMain)
        add_test(NAME PROI_tests COMMAND PROI_tests)
    endif ()
endif ()