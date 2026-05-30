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
            #-Wconversion -Wsign-conversion -Wshadow -Wold-style-cast
    )
endif ()

add_library(hotel_logic
    # booking
    include/booking/Bill.h
    include/booking/Reservation.h
    include/booking/booking.hpp
    src/booking/Bill.cpp
    src/booking/Reservation.cpp
    # guests
    include/guests/VIP.hpp
    include/guests/Guest.hpp
    include/guests/Business.hpp
    include/guests/Family.hpp
    src/guests/VIP.cpp
    src/guests/Business.cpp
    src/guests/Guest.cpp
    src/guests/Family.cpp
    # hotel
    include/hotel/Hotel.h
    src/hotel/Hotel.cpp
    # events
    include/events/FireAlarm.h
    include/events/PowerOutage.h
    include/events/events.hpp
    include/events/EventGenerator.h
    include/events/HotelEvent.h
    src/events/EventGenerator.cpp
    src/events/HotelEvent.cpp
    src/events/PowerOutage.cpp
    src/events/FireAlarm.cpp
    # meal
    include/meal/MealTypes.hpp
    src/meal/MealTypes.cpp
    # logger
    include/logger/Logger.hpp
    src/logger/Logger.cpp
    # config
    include/file_parse.hpp
    include/config/file_parse.hpp
    include/config/sim_config_parse.hpp
    include/config/config_loader.hpp
    include/config/hotel_loader.hpp
    src/config/file_parse.cpp
    src/config/hotel_loader.cpp
    src/config/config_loader.cpp
    src/config/sim_config_parse.cpp
    # rooms
    include/rooms/Room.h
    include/rooms/SingleRoom.h
    include/rooms/DoubleRoom.h
    include/rooms/TripleRoom.h
    include/rooms/Apartment.h
    include/rooms/rooms.hpp
    src/rooms/Room.cpp
    src/rooms/SingleRoom.cpp
    src/rooms/DoubleRoom.cpp
    src/rooms/TripleRoom.cpp
    src/rooms/Apartment.cpp
    # simulation
    include/simulation/Simulation.hpp
    src/simulation/Simulation.cpp
    # staff
    include/staff/types.hpp
    include/staff/Staff.hpp
    include/staff/Receptionist.hpp
    include/staff/Housekeeper.hpp
    include/staff/Chef.hpp
    include/staff/Waiter.hpp
    include/staff/MaintenanceWorker.hpp
    src/staff/Task.cpp
    src/staff/Staff.cpp
    src/staff/Receptionist.cpp
    src/staff/Housekeeper.cpp
    src/staff/Chef.cpp
    src/staff/Waiter.cpp
    src/staff/MaintenanceWorker.cpp
    src/staff/staff_loader.cpp
)

target_include_directories(hotel_logic PUBLIC include)
target_compile_options(hotel_logic PRIVATE -Wno-error)


# Executables
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
        add_executable(UnitTests
            tests/test_staff.cpp
            tests/test_config_loader.cpp
            tests/test_staff_loader.cpp
            tests/test_hotel_loader.cpp
            tests/test_simulation_date.cpp
            tests/test_event_log.cpp
            tests/test_housekeeping.cpp
            tests/test_maintenance.cpp
            tests/test_hotel_event.cpp
        )
        target_link_libraries(UnitTests PRIVATE hotel_logic Catch2::Catch2WithMain)
        add_test(NAME UnitTests COMMAND UnitTests)
    endif ()

    if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/app/tests.cpp")
        add_executable(PROI_tests app/tests.cpp)
        target_link_libraries(PROI_tests hotel_logic Catch2::Catch2WithMain)
        add_test(NAME PROI_tests COMMAND PROI_tests)
    endif ()
endif ()
