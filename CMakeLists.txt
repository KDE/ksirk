project(ksirk)

if(NOT COMPILING_KDEGAMES)
	find_package(KDE4 REQUIRED)
	include(KDE4Defaults)
	include(MacroLibrary)
	find_package(KDEGames REQUIRED)
	find_package(QCA2 REQUIRED)

	add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
	add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)
	include_directories(${KDEGAMES_INCLUDE_DIRS} ${KDE4_INCLUDES} ${QDBUS_INCLUDE_DIRS})

	macro_log_feature(QCA2_FOUND "QCA2" "Qt Cryptographic Architecture" "http://delta.affinix.com/qca" TRUE "2.0.0" "Needed to build Ksirk Jabber support's GroupWise and Jabberprotocols.")
	macro_display_feature_log()
endif(NOT COMPILING_KDEGAMES)

KDE4_NO_ENABLE_FINAL(ksirk)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")

remove_definitions(-DQT_NO_STL) 

add_subdirectory(ksirk)
add_subdirectory(ksirkskineditor)
add_subdirectory(doc)
