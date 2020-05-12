cmake_minimum_required(VERSION 2.8.12)
project(obs-command-source)

set(CMAKE_PREFIX_PATH "${QTDIR}")
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (WIN32 OR APPLE)
    include(external/FindLibobs.cmake)
endif()

find_package(LibObs REQUIRED)
find_package(Qt5 REQUIRED Core Widgets)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	set(ARCH 64)
else()
	set(ARCH 32)
endif()

set(command-source_SOURCES obs-command-source.c)

# set(command-source_HEADERS )

add_library(command-source MODULE ${command-source_SOURCES})


include_directories(
	"${LIBOBS_INCLUDE_DIR}/../UI/obs-frontend-api"
	${Qt5Core_INCLUDES}
	${Qt5Widgets_INCLUDES})



target_link_libraries(command-source
	libobs
	Qt5::Core
	Qt5::Widgets)

# OSX
if(APPLE)
	set_target_properties(command-source PROPERTIES PREFIX "")
	target_link_libraries(command-source
		"${OBS_FRONTEND_LIB}")
endif()

# Linux
if(UNIX AND NOT APPLE)
	include(GNUInstallDirs)

	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -mtune=core2 -Ofast")

	set_target_properties(command-source PROPERTIES PREFIX "")
	target_link_libraries(command-source obs-frontend-api)

	file(GLOB locale_files data/locale/*.ini)

	install(TARGETS command-source
                LIBRARY DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}/obs-plugins)

	install(FILES ${locale_files}
		DESTINATION "${CMAKE_INSTALL_FULL_DATAROOTDIR}/obs/obs-plugins/command-source/locale")
endif()
