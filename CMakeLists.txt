project(ksirk)
find_package(KDE4 REQUIRED)
include (KDE4Defaults)
include (MacroLibrary)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules )

#add_definitions (${QT_DEFINITIONS} -DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS ${KDE4_DEFINITIONS} -DHAVE_CONFIG_H=1 -fexceptions)
add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS} -DHAVE_CONFIG_H=1 -fexceptions)

remove_definitions(-DQT_NO_STL) 

link_directories (${KDE4_LIB_DIR})

include_directories (${QDBUS_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES})

# Create config.h
configure_file (config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h )


include (ConfigureChecks.cmake)
configure_file (config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h )


MESSAGE(STATUS "currently finishing the porting of KsirK to kde4")
add_subdirectory(ksirk)

macro_display_feature_log()
