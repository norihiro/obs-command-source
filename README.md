Dummy Source to Execute Command
===============================

This plugin provides a dummy source to execute arbitrary command when scene is switched.

## Current Features

* Execute command at following events.
  * Show (the scene is shown in either preview or program)
  * Hide (the scene is hidden so that no longer shown in neither preview nor program)
  * Activate (the scene goes to the program)
  * Deactivate (the scene goes from the program)
  * Show in preview (the scene goes to the preview)
  * Hide from preview (the scene goes from the preview)

## Possible Usage

* Implementing custom tally lights.
* Control PTZ cameras by switching the scene.
  You may combine with CURL to send some commands.
* Start and stop a daemon program required for the scene.
* Trigger other operations through websocket at the event.
  A helper script is available at `tools/request-websocket.py`.
* Not limited to above.

## Caution

Since this plugin execute arbitrary command, user need consider security concern.
For example, combining with `obs-websocket` plugin,
remote user could change property through the websocket interface so that arbitrary command can be executed.

## Planned Features

* Support Mac OS X
  Currently this plugin is only tested on Linux and Windows.
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
# Copy obs-command-source.so to the obs-plugins folder
```
