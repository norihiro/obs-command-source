Dummy Source to Execute Command
===============================

This plugin provides a dummy source to execute arbitrary command when scene is switched.

## Current Features

* Start a command at following events.
  * Show (the source is shown in either preview or program)
  * Hide (the source is hidden so that no longer shown in neither preview nor program)
  * Activate (the source goes to the program)
  * Deactivate (the source goes from the program)
  * Show in preview (the source goes to the preview)
  * Hide from preview (the source goes from the preview)
* Optionally, kill the created process at these conditions. This feature is not available for Windows as of now.
  * When hiding, kill the process created at shown.
  * When deactivating, kill the process created at activated.
  * When hiding from the preview, kill the process created at preview.

## Possible Usage

* Implementing custom tally lights.
* Control PTZ cameras by switching the scene.
  You may combine with CURL to send some commands.
* Start and stop a daemon program required for the scene.
* Trigger other operations through websocket at the event.
  A helper script is available at `tools/request-websocket.py`.
  - Start or stop your streaming and recording.
* Not limited to above.

## Caution

Since this plugin execute arbitrary command, user need consider security concern.
For example, combining with `obs-websocket` plugin,
remote user could change property through the websocket interface so that arbitrary command can be executed.

## Planned Features

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
