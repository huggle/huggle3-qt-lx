#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd -- "${SCRIPT_DIR}/.." && pwd)"
SOURCE_DIR="${ROOT_DIR}/src"

if [[ "$(uname -s)" != "Darwin" ]]; then
    echo "macOS packaging must run on macOS" >&2
    exit 1
fi

missing_submodules=()
for path in \
    src/3rd/libirc/CMakeLists.txt \
    src/3rd/yaml-cpp/CMakeLists.txt \
    src/extensions/extension-scoring/CMakeLists.txt \
    src/extensions/extension-thanks/CMakeLists.txt \
    src/extensions/enwiki/CMakeLists.txt \
    src/extensions/mass-delivery/CMakeLists.txt \
    src/extensions/extension-splitter-helper/CMakeLists.txt \
    src/extensions/extension-mass-delete/CMakeLists.txt \
    src/extensions/extension-flow/CMakeLists.txt
do
    [[ -f "${ROOT_DIR}/${path}" ]] || missing_submodules+=("${path}")
done
if (( ${#missing_submodules[@]} > 0 )); then
    printf 'Missing required submodule: %s\n' "${missing_submodules[@]}" >&2
    echo "Run: git submodule update --init --recursive" >&2
    exit 1
fi

find_qt_prefix()
{
    if [[ -n "${1:-}" ]]; then
        printf '%s\n' "$1"
    elif [[ -n "${QT_ROOT_DIR:-}" ]]; then
        printf '%s\n' "${QT_ROOT_DIR}"
    elif command -v qtpaths6 >/dev/null 2>&1; then
        qtpaths6 --query QT_INSTALL_PREFIX
    elif command -v qtpaths >/dev/null 2>&1; then
        qtpaths --query QT_INSTALL_PREFIX
    else
        echo "Unable to find Qt 6; pass its prefix as the first argument or set QT_ROOT_DIR" >&2
        return 1
    fi
}

QT_PREFIX="$(find_qt_prefix "${1:-}")"
MACDEPLOYQT="${QT_PREFIX}/bin/macdeployqt"
if [[ ! -x "${MACDEPLOYQT}" ]]; then
    echo "Unable to find macdeployqt at ${MACDEPLOYQT}" >&2
    exit 1
fi
QTPATHS="${QT_PREFIX}/bin/qtpaths"
if [[ ! -x "${QTPATHS}" ]]; then
    QTPATHS="${QT_PREFIX}/bin/qtpaths6"
fi
if [[ ! -x "${QTPATHS}" || "$("${QTPATHS}" --qt-version)" != "6.9.2" ]]; then
    echo "macOS release packaging requires the official Qt 6.9.2 distribution" >&2
    exit 1
fi

QT_CORE="${QT_PREFIX}/lib/QtCore.framework/Versions/A/QtCore"
QT_ARCHITECTURES="$(lipo -archs "${QT_CORE}")"
if [[ " ${QT_ARCHITECTURES} " != *" arm64 "* ||
      " ${QT_ARCHITECTURES} " != *" x86_64 "* ]]; then
    echo "Qt 6.9.2 must contain both arm64 and x86_64 architectures" >&2
    exit 1
fi

BUILD_ROOT="${HUGGLE_BUILD_DIR:-${ROOT_DIR}/release/macos-universal}"
BUILD_DIR="${BUILD_ROOT}/build"
STAGE_DIR="${BUILD_ROOT}/stage"
OUTPUT_DIR="${HUGGLE_OUTPUT_DIR:-${ROOT_DIR}/release/artifacts}"
PARALLEL="${CMAKE_BUILD_PARALLEL_LEVEL:-$(sysctl -n hw.ncpu)}"

cmake -S "${SOURCE_DIR}" -B "${BUILD_DIR}" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_BINDIR:PATH=bin \
    -DCMAKE_INSTALL_DATADIR:PATH=share \
    -DCMAKE_INSTALL_LIBDIR:PATH=lib \
    -DCMAKE_INSTALL_PREFIX:PATH=/usr/local \
    -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" \
    -DCMAKE_PREFIX_PATH="${QT_PREFIX}" \
    -DQT5_BUILD=OFF \
    -DQT6_BUILD=ON \
    -DWEB_ENGINE=ON \
    -DHUGGLE_EXT=ON \
    -DHUGGLE_TEST=ON \
    -DCMAKE_DISABLE_FIND_PACKAGE_YAML-CPP=ON \
    -DYAML_BUILD_SHARED_LIBS=OFF

cmake --build "${BUILD_DIR}" --parallel "${PARALLEL}"
ctest --test-dir "${BUILD_DIR}" --output-on-failure

cmake -E remove_directory "${STAGE_DIR}"
cmake --install "${BUILD_DIR}" --prefix "${STAGE_DIR}"

APP_BUNDLE="${STAGE_DIR}/huggle.app"
FRAMEWORKS_DIR="${APP_BUNDLE}/Contents/Frameworks"
PLUGINS_DIR="${APP_BUNDLE}/Contents/PlugIns"

if [[ ! -d "${APP_BUNDLE}" ]]; then
    echo "CMake did not install ${APP_BUNDLE}" >&2
    exit 1
fi

mkdir -p "${FRAMEWORKS_DIR}" "${PLUGINS_DIR}"
shopt -s nullglob

PROJECT_LIBRARIES=("${STAGE_DIR}/lib/"*.dylib)
if (( ${#PROJECT_LIBRARIES[@]} == 0 )); then
    echo "No Huggle libraries were installed in ${STAGE_DIR}/lib" >&2
    exit 1
fi
cp -R -p "${PROJECT_LIBRARIES[@]}" "${FRAMEWORKS_DIR}/"

COMPILED_EXTENSIONS=("${STAGE_DIR}/share/huggle/extensions/"*.dylib)
SCRIPT_EXTENSIONS=("${STAGE_DIR}/share/huggle/extensions/"*.js)
if (( ${#COMPILED_EXTENSIONS[@]} == 0 || ${#SCRIPT_EXTENSIONS[@]} == 0 )); then
    echo "Huggle extensions were not installed completely" >&2
    exit 1
fi
cp -R -p "${COMPILED_EXTENSIONS[@]}" "${PLUGINS_DIR}/"

DEPLOY_ARGS=("${APP_BUNDLE}" "-always-overwrite"
             "-libpath=${FRAMEWORKS_DIR}" "-libpath=${QT_PREFIX}/lib")
for extension in "${PLUGINS_DIR}/"*.dylib; do
    DEPLOY_ARGS+=("-executable=${extension}")
done
"${MACDEPLOYQT}" "${DEPLOY_ARGS[@]}"

cp -R -p "${SCRIPT_EXTENSIONS[@]}" "${PLUGINS_DIR}/"
codesign --force --deep --sign - "${APP_BUNDLE}"
"${SCRIPT_DIR}/verify-app.sh" "${APP_BUNDLE}"

VERSION="$(plutil -extract CFBundleShortVersionString raw "${APP_BUNDLE}/Contents/Info.plist")"
mkdir -p "${OUTPUT_DIR}"
DMG_PATH="${OUTPUT_DIR}/huggle_${VERSION}_universal.dmg"
rm -f "${DMG_PATH}"
DMG_CREATED=false
for attempt in 1 2 3; do
    if hdiutil create -volname "Huggle ${VERSION}" -srcfolder "${APP_BUNDLE}" \
        -format UDZO -ov "${DMG_PATH}"; then
        DMG_CREATED=true
        break
    fi
    if (( attempt < 3 )); then
        echo "DMG creation failed; retrying (${attempt}/3)" >&2
        rm -f "${DMG_PATH}"
        sleep 5
    fi
done
if [[ "${DMG_CREATED}" != true ]]; then
    echo "Unable to create ${DMG_PATH} after 3 attempts" >&2
    exit 1
fi

printf 'Created %s\n' "${DMG_PATH}"
