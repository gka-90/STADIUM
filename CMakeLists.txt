
cmake_minimum_required(VERSION 3.16)
project(stadium_booking LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Sources in src/
file(GLOB SRC_FILES src/*.cpp)

add_executable(stadium_server ${SRC_FILES})

# Make sure we can find standalone Asio (Homebrew: /opt/homebrew/include on Apple Silicon,
# /usr/local/include on Intel). Also define ASIO_STANDALONE for Crow.
find_path(ASIO_INCLUDE_DIR
  NAMES asio.hpp
  PATHS /opt/homebrew/include /usr/local/include
)

if (ASIO_INCLUDE_DIR)
  message(STATUS "Found standalone Asio at: ${ASIO_INCLUDE_DIR}")
  target_include_directories(stadium_server PRIVATE ${ASIO_INCLUDE_DIR})
  target_compile_definitions(stadium_server PRIVATE ASIO_STANDALONE)
else()
  message(WARNING "Standalone Asio not found; trying Boost.Asio (CROW_USE_BOOST).")
  find_package(Boost 1.70 QUIET COMPONENTS system)
  if (Boost_FOUND)
    target_compile_definitions(stadium_server PRIVATE CROW_USE_BOOST)
    target_include_directories(stadium_server PRIVATE ${Boost_INCLUDE_DIRS})
    target_link_libraries(stadium_server PRIVATE Boost::system)
  else()
    message(FATAL_ERROR "Neither standalone Asio nor Boost.Asio found. Install with 'brew install asio' or 'brew install boost'.")
  endif()
endif()


# Include headers from src/ (for Stadia.h, crow.h, etc.)
target_include_directories(stadium_server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# SQLite3
find_package(SQLite3 QUIET)
if (SQLite3_FOUND)
    message(STATUS "Found SQLite3 via CMake: ${SQLite3_VERSION}")
    target_link_libraries(stadium_server PRIVATE SQLite::SQLite3)
else()
    message(WARNING "CMake couldn't find SQLite3 package; falling back to linking -lsqlite3 directly.")
    target_link_libraries(stadium_server PRIVATE sqlite3)
endif()

# Threads
find_package(Threads REQUIRED)
target_link_libraries(stadium_server PRIVATE Threads::Threads)

# Boost (optional for Crow)
find_package(Boost 1.70 QUIET COMPONENTS system)
if (Boost_FOUND)
    target_link_libraries(stadium_server PRIVATE Boost::system)
endif()

# Warnings
if (MSVC)
    target_compile_options(stadium_server PRIVATE /W4)
else()
    target_compile_options(stadium_server PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Copy public/ folder to build dir for convenience
add_custom_command(TARGET stadium_server POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/public $<TARGET_FILE_DIR:stadium_server>/public
)
