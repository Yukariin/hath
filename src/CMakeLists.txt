set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11")

find_package(Threads REQUIRED)

include_directories("${PROJECT_SOURCE_DIR}/lib/http-parser")
set(http_parser_SRCS "${PROJECT_SOURCE_DIR}/lib/http-parser/http_parser.c")

include_directories("${PROJECT_SOURCE_DIR}/lib/sqlite")
set(sqlite_SRCS "${PROJECT_SOURCE_DIR}/lib/sqlite/sqlite3.c")

include_directories("${PROJECT_SOURCE_DIR}/lib/asio/asio/include")
add_definitions("-DASIO_STANDALONE")

include_directories("${PROJECT_SOURCE_DIR}/lib/system/include")
include_directories("${PROJECT_SOURCE_DIR}/lib/filesystem/include")
add_definitions("-DBOOST_SYSTEM_NO_DEPRECATED -DBOOST_FILESYSTEM_NO_DEPRECATED")

file(GLOB_RECURSE fs_SRCS ${PROJECT_SOURCE_DIR}/lib/system/src/*.cpp ${PROJECT_SOURCE_DIR}/lib/filesystem/src/*.cpp )
add_library(fs ${fs_SRCS})

file(GLOB_RECURSE hath_SRCS *.cpp)
add_executable(hath ${hath_SRCS} ${http_parser_SRCS} ${sqlite_SRCS})
target_link_libraries(hath ${CMAKE_THREAD_LIBS_INIT} ${CMAKE_DL_LIBS} fs)
