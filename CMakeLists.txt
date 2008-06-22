project(ksirk)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")

remove_definitions(-DQT_NO_STL) 

add_subdirectory(ksirk)

