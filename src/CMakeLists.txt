set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11")

include_directories("${PROJECT_SOURCE_DIR}/lib/asio/asio/include")
add_definitions("-DASIO_STANDALONE")

file(GLOB_RECURSE hath_SRCS *.cpp *.h)

add_executable(hath ${hath_SRCS})
target_link_libraries(hath)
