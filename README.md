Dummy Source to Execute Command
===============================

This plugin provides a dummy source to execute arbitral command when scene is switched.

Current Features
----------------

* Execute command at following events.
  * Show (the scene is shown in preview)
  * Hide (the scene is hidden from preview)
  * Activate (the scene goes to the program)
  * Deactivate (the scene goes from the program)

Possible Usage
--------------

* Implementing custom tally lights.
* Control PTZ cameras by switching the scene.
  You may combine with CURL to send some commands.
* Start and stop a daemon program required for the scene.
* Not limited to above.

Planned Features
----------------

* Environmental variables or parametric arguments to pass more information
  * Scene name
  * Source name
  * Other sources in the same scene
* Support Mac OS X and Windows
  Currently this plugin is only tested on Linux.
  It should work on Mac OS X but Windows need more implementation.

Build
-----

You can either build the plugin as a standalone project or integrate it
into the build of OBS Studio.

Building it as a standalone project follows the standard cmake approach.
Create a new directory and run `cmake ... <path_to_source>` for whichever
build system you use. You may have to set the `CMAKE_INCLUDE_PATH`
environment variable to the location of libobs's header files if cmake
does not find them automatically. You may also have to set the
`CMAKE_LIBRARY_PATH` environment variable to the location of the libobs
binary if cmake does not find it automatically.

To integrate the plugin into the OBS Studio build put the source into a
subdirectory of the `plugins` folder of OBS Studio and add it to the
CMakeLists.txt.
