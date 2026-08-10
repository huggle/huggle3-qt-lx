# macOS builds

Huggle is packaged for macOS with Qt 6 as a universal application containing
native Apple Silicon (`arm64`) and Intel (`x86_64`) binaries.

## Requirements

- macOS 14 or newer for producing release artifacts
- Xcode 15 command-line tools or newer
- CMake
- Git with the repository submodules initialized
- The official Qt 6.9.2 `clang_64` distribution with WebEngine, WebChannel,
  Multimedia, Positioning, and SerialPort

Initialize the dependencies before the first build:

```sh
git submodule update --init --recursive
```

## Build and package

Run the release script and pass the Qt installation prefix if Qt is not
discoverable through `QT_ROOT_DIR`, `qtpaths6`, or `qtpaths`.

```sh
./MacOS/release.sh /path/to/Qt/6.9.2/macos
```

Release packaging intentionally uses the same official Qt distribution as CI.
Homebrew Qt is not supported for packaging because its split framework layout
is incompatible with `macdeployqt`. It remains suitable for ordinary builds.

The script configures a release build with extensions and tests enabled,
runs the test suite, deploys Qt into the application bundle, validates the
bundle, and writes the result to:

```text
release/artifacts/huggle_<version>_universal.dmg
```

The following environment variables can override the defaults:

- `HUGGLE_BUILD_DIR`: build and staging directory.
- `HUGGLE_OUTPUT_DIR`: destination for the DMG.
- `CMAKE_BUILD_PARALLEL_LEVEL`: number of parallel build jobs.

Qt 6.9 supports deployment to macOS 12 or newer.

## Distribution policy

`macdeployqt` applies an ad-hoc signature so the deployed ARM binaries remain
valid after their dependencies are rewritten. Huggle does not use a paid
Developer ID certificate and the DMGs are not Apple-notarized.

GitHub Actions builds and validates the universal application and publishes
the DMG as a workflow artifact.
