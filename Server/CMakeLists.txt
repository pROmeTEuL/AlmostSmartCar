cmake_minimum_required(VERSION 3.14)

project(Server LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Network REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Network REQUIRED)

find_package(Threads REQUIRED)

include(FetchContent)

find_package(pigpio QUIET)
if (NOT TARGET pigpio::core)
    message(STATUS "pigpio was not found. Fetching from git")
    FetchContent_Declare(
        pigpio
        GIT_REPOSITORY https://github.com/bog-dan-ro/pigpio.git
        GIT_TAG master
        CMAKE_ARGS -DBUILD_SHARED_LIBS=OFF -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        )
    FetchContent_MakeAvailable(pigpio)
endif()

add_executable(Server
  main.cpp
  server.cpp
  server.h
  sonic.cpp
  sonic.h
  LiquidCrystal.cpp
  LiquidCrystal.h
)
target_link_libraries(Server Qt${QT_VERSION_MAJOR}::Network pigpio::core Threads::Threads)
