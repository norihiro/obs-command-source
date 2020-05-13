cmake_minimum_required(VERSION 2.8.12)
project(obs-command-source)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT OBS_OUTPUT_DIR)
	if (WIN32 OR APPLE)
	    include(external/FindLibobs.cmake)
	endif()
endif()

find_package(LibObs REQUIRED)

set(command-source_SOURCES obs-command-source.c)

add_library(command-source MODULE ${command-source_SOURCES})

include_directories(
	"${LIBOBS_INCLUDE_DIR}/../UI/obs-frontend-api"
	)

target_link_libraries(command-source
	libobs
	)

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

if(OBS_OUTPUT_DIR)
	install_obs_plugin_with_data(command-source data)
endif()
