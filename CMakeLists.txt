cmake_minimum_required(VERSION 2.8.12)
project(command-source)
set(CMAKE_PROJECT_VERSION 0.1.2)

set(PLUGIN_AUTHOR "Norihiro Kamae")
set(MACOS_BUNDLEID "net.nagater.obs-command-source")
set(LINUX_MAINTAINER_EMAIL "norihiro@nagater.net")

# TAKE NOTE: No need to edit things past this point

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT OBS_OUTPUT_DIR)
	set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
endif()

if (WIN32 OR APPLE)
	include(external/FindLibobs.cmake)
endif()

find_package(LibObs REQUIRED)

configure_file(
	installer/installer-Windows.iss.in
	../installer/installer-Windows.generated.iss
)

configure_file(
	ci/ci_includes.sh.in
	../ci/ci_includes.generated.sh
)
configure_file(
	ci/ci_includes.cmd.in
	../ci/ci_includes.generated.cmd
)

set(command-source_SOURCES obs-command-source.c)

add_library(command-source MODULE ${command-source_SOURCES})

include_directories(
	"${LIBOBS_INCLUDE_DIR}/../UI/obs-frontend-api"
	)

target_link_libraries(command-source
	libobs
	)

# --- Windows-specific build settings and tasks ---
if(WIN32)
	if(NOT DEFINED OBS_FRONTEND_LIB)
		set(OBS_FRONTEND_LIB "OBS_FRONTEND_LIB-NOTFOUND" CACHE FILEPATH "OBS frontend library")
		message(FATAL_ERROR "Could not find OBS Frontend API\'s library !")
	endif()

	# Enable Multicore Builds and disable FH4 (to not depend on VCRUNTIME140_1.DLL when building with VS2019)
	if (MSVC)
		add_definitions(/MP /d2FH4-)
	endif()

	if(CMAKE_SIZEOF_VOID_P EQUAL 8)
		set(ARCH_NAME "64bit")
		set(OBS_BUILDDIR_ARCH "build64")
	else()
		set(ARCH_NAME "32bit")
		set(OBS_BUILDDIR_ARCH "build32")
	endif()

	include_directories(
		"${LIBOBS_INCLUDE_DIR}/../${OBS_BUILDDIR_ARCH}/UI"
	)

	target_link_libraries(${CMAKE_PROJECT_NAME}
		"${OBS_FRONTEND_LIB}"
	)

	# --- Release package helper ---
	# The "release" folder has a structure similar OBS' one on Windows
	set(RELEASE_DIR "${PROJECT_SOURCE_DIR}/release")

    add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${RELEASE_DIR}/data/obs-plugins/${CMAKE_PROJECT_NAME}" "${RELEASE_DIR}/obs-plugins/${ARCH_NAME}"

        COMMAND "${CMAKE_COMMAND}" -E copy_directory "${PROJECT_SOURCE_DIR}/data" "${RELEASE_DIR}/data/obs-plugins/${CMAKE_PROJECT_NAME}"

        COMMAND "${CMAKE_COMMAND}" -E copy "$<TARGET_FILE:${CMAKE_PROJECT_NAME}>" "${RELEASE_DIR}/obs-plugins/${ARCH_NAME}/"
    )

endif()

# --- Linux-specific build settings and tasks ---
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

# -- OS X specific build settings and tasks --
if(APPLE)
	set_target_properties(command-source PROPERTIES PREFIX "")
	target_link_libraries(command-source
		"${OBS_FRONTEND_LIB}")
endif()

if(OBS_OUTPUT_DIR)
	install_obs_plugin_with_data(command-source data)
endif()
