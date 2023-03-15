# Environment variables

This is an experimental feature for Linux and macOS. The behavior might change in future releases.
Environment variables are set when calling a process.

### `OBS_CURRENT_SCENE`
Name of the current scene.
In the command for `Activated` and `Deactivated`,
the new scene name will be set to this variable.

### `OBS_PREVIEW_SCENE`
Name of the current scene in preview.

### `OBS_SOURCE_NAME`
Name of the command source.
It is useful to set the visibility of a scene item by a combination of `OBS_CURRENT_SCENE` or `OBS_PREVIEW_SCENE`.

### `OBS_TRANSITION_DURATION`
Duration in milliseconds [ms] for the current transition.
