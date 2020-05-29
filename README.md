Dummy Source to Execute Command
===============================

This plugin provides a dummy source to execute arbitrary command when scene is switched.

## Current Features

* Execute command at following events.
  * Show (the scene is shown in either preview or program)
  * Hide (the scene is hidden so that no longer shown in neither preview nor program)
  * Activate (the scene goes to the program)
  * Deactivate (the scene goes from the program)

## Possible Usage

* Implementing custom tally lights.
* Control PTZ cameras by switching the scene.
  You may combine with CURL to send some commands.
* Start and stop a daemon program required for the scene.
* Trigger other operations through websocket at the event.
  A helper script is available at `tools/request-websocket.py`.
* Not limited to above.

## Planned Features

* Environmental variables or parametric arguments to pass more information
  * Scene name
  * Source name
  * Other sources in the same scene
* Support Mac OS X and Windows
  Currently this plugin is only tested on Linux.
  It should work on Mac OS X but Windows need more implementation.
* Icon

## Build

### Linux
```
git clone https://github.com/norihiro/obs-command-source.git
cd obs-command-source
mkdir build && cd build
cmake \
	-DLIBOBS_INCLUDE_DIR="<path to the libobs sub-folder in obs-studio's source code>" \
	-DCMAKE_INSTALL_PREFIX=/usr ..
make -j4
sudo make install
```

### OS X
```
git clone https://github.com/norihiro/obs-command-source.git
cd obs-command-source
mkdir build && cd build
cmake \
	-DLIBOBS_INCLUDE_DIR=<path to the libobs sub-folder in obs-studio's source code> \
	-DLIBOBS_LIB=<path to libobs.0.dylib> \
	-DOBS_FRONTEND_LIB=<path to libobs-frontend-api.dylib> \
	..
make -j4
# Copy command-source.so to the obs-plugins folder
```

### Integrate into OBS Studio
```
cd obs-studio/plugins
git clone https://github.com/norihiro/obs-command-source.git
echo 'add_subdirectory(obs-command-source)' >> CMakeLists.txt
# Build and install OBS Studio.
```
