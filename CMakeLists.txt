cmake_minimum_required(VERSION 2.8.12)

project(obs-command-source)

# Standalone build
if(NOT OBS_OUTPUT_DIR)
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
    include(ObsHelpers)

    set(CMAKE_C_STANDARD 11)

    find_path(LIBOBS_INCLUDE_DIR
        obs-module.h)
    find_library(OBS_LIBRARIES
        NAMES libobs obs)
# Build as submodule of obs-studio
else()
	set(LIBOBS_INCLUDE_DIR
		../../libobs)
	set(OBS_LIBRARIES
		libobs)
endif()


include_directories(
	${LIBOBS_INCLUDE_DIR}
	)

set(command-source_SOURCES obs-command-source.c)

add_library(command-source MODULE ${command-source_SOURCES})

target_link_libraries(command-source
	${OBS_LIBRARIES}
	)

install_obs_plugin_with_data(command-source data)
