# macOS builds

Huggle is packaged for macOS with Qt 6. The default release is a universal
application containing native Apple Silicon (`arm64`) and Intel (`x86_64`)
binaries. Smaller architecture-specific DMGs can also be produced.

## Requirements

- macOS 14 or newer for producing release artifacts
- Xcode 15 command-line tools or newer
- CMake
- Git with the repository submodules initialized
- The official Qt `clang_64` distribution selected by `HUGGLE_QT_VERSION`
  (6.9.2 by default), with WebEngine, WebChannel, Multimedia, and Positioning

Initialize the dependencies before the first build:

```sh
git submodule update --init --recursive
```

## Build and package

Run the release script and pass the Qt root directory and version if Qt is not
discoverable through `QT_ROOT_DIR`, `qtpaths6`, or `qtpaths`.

```sh
./MacOS/release.sh --qt-prefix ~/Qt --qt-version 6.9.2
```

To build only one architecture, add `--arm` or `--intel`:

```sh
./MacOS/release.sh --arm --qt-prefix ~/Qt --qt-version 6.9.2
./MacOS/release.sh --intel --qt-prefix ~/Qt --qt-version 6.9.2
```

To build all release DMGs in one run, use `--all`:

```sh
./MacOS/release.sh --all --qt-prefix ~/Qt --qt-version 6.9.2
```

For custom builds or non-standard Qt layouts, pass the complete Qt installation
prefix explicitly with `--qt-full-path`:

```sh
./MacOS/release.sh --qt-full-path /path/to/Qt/6.9.2/macos
```

Release packaging intentionally uses the same official Qt distribution as CI.
Homebrew Qt is not supported for packaging because its split framework layout
is incompatible with `macdeployqt`. It remains suitable for ordinary builds.

The script configures a release build with extensions and tests enabled,
runs the test suite, deploys Qt into the application bundle, validates the
bundle, and writes the result to:

```text
release/artifacts/huggle_<version>_universal.dmg
release/artifacts/huggle_<version>_arm.dmg
release/artifacts/huggle_<version>_intel.dmg
```

The following environment variables can override the defaults:

- `HUGGLE_BUILD_DIR`: build and staging directory. When `--all` is used with
  this override, each variant uses a subdirectory below `HUGGLE_BUILD_DIR`.
- `HUGGLE_OUTPUT_DIR`: destination for the DMG.
- `HUGGLE_QT_VERSION`: required Qt version (defaults to the CI version, 6.9.2).
- `CMAKE_BUILD_PARALLEL_LEVEL`: number of parallel build jobs.

Run `./MacOS/release.sh --help` for the complete list of script options.

Qt 6.9 supports deployment to macOS 12 or newer.

## Distribution policy

`macdeployqt` applies an ad-hoc signature so the deployed ARM binaries remain
valid after their dependencies are rewritten. Huggle does not use a paid
Developer ID certificate and the DMGs are not Apple-notarized.

GitHub Actions builds and validates the universal application and publishes
the DMG as a workflow artifact.
