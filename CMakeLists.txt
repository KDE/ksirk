project(ksirk)

add_definitions(${KDE4_ENABLE_EXCEPTIONS})

remove_definitions(-DQT_NO_STL) 

#MESSAGE(STATUS "currently finishing the porting of KsirK to kde4")
add_subdirectory(ksirk)

