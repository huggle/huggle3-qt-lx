#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd -- "${SCRIPT_DIR}/.." && pwd)"
SOURCE_DIR="${ROOT_DIR}/src"
DEFAULT_QT_VERSION="6.9.2"
QT_VERSION="${HUGGLE_QT_VERSION:-${DEFAULT_QT_VERSION}}"
QT_ROOT_ARG=""
QT_FULL_PATH_ARG=""

usage()
{
    cat <<EOF
Usage: $(basename "$0") [options] [QT_FULL_PATH]

Build, test, and package a universal macOS Huggle release DMG.

Options:
  -h, --help                 Show this help text and exit.
      --qt-prefix PATH       Qt root directory. The Qt prefix is resolved as
                             PATH/<qt-version>/macos.
      --qt-full-path PATH    Full Qt installation prefix containing
                             bin/macdeployqt. This is equivalent to the
                             optional QT_FULL_PATH positional argument.
      --qt-version VERSION   Required Qt version. Overrides HUGGLE_QT_VERSION
                             and the default ${DEFAULT_QT_VERSION}.

Environment:
  QT_ROOT_DIR                     Full Qt installation prefix if Qt path options
                                  and QT_FULL_PATH are omitted.
  HUGGLE_QT_VERSION               Required Qt version, default ${DEFAULT_QT_VERSION}.
  HUGGLE_BUILD_DIR                Build and staging root, default
                                  release/macos-universal.
  HUGGLE_OUTPUT_DIR               DMG output directory, default
                                  release/artifacts.
  CMAKE_BUILD_PARALLEL_LEVEL      Number of parallel build jobs, default
                                  the host CPU count.

Examples:
  ./MacOS/release.sh --qt-prefix ~/Qt --qt-version 6.9.2
  ./MacOS/release.sh --qt-full-path /path/to/Qt/6.9.2/macos
  ./MacOS/release.sh /path/to/Qt/6.9.2/macos
EOF
}

set_qt_root()
{
    if [[ -n "${QT_FULL_PATH_ARG}" ]]; then
        echo "--qt-prefix cannot be used with --qt-full-path or positional QT_FULL_PATH" >&2
        exit 2
    fi
    if [[ -n "${QT_ROOT_ARG}" ]]; then
        echo "Only one Qt root may be specified" >&2
        exit 2
    fi
    QT_ROOT_ARG="$1"
}

set_qt_full_path()
{
    if [[ -n "${QT_ROOT_ARG}" ]]; then
        echo "--qt-full-path cannot be used with --qt-prefix" >&2
        exit 2
    fi
    if [[ -n "${QT_FULL_PATH_ARG}" ]]; then
        echo "Only one full Qt path may be specified" >&2
        exit 2
    fi
    QT_FULL_PATH_ARG="$1"
}

while (( $# > 0 )); do
    case "$1" in
        -h|--help)
            usage
            exit 0
            ;;
        --qt-prefix)
            if (( $# < 2 )); then
                echo "--qt-prefix requires a path" >&2
                exit 2
            fi
            if [[ -z "$2" ]]; then
                echo "--qt-prefix requires a non-empty path" >&2
                exit 2
            fi
            set_qt_root "$2"
            shift 2
            ;;
        --qt-prefix=*)
            qt_root="${1#*=}"
            if [[ -z "${qt_root}" ]]; then
                echo "--qt-prefix requires a non-empty path" >&2
                exit 2
            fi
            set_qt_root "${qt_root}"
            shift
            ;;
        --qt-full-path)
            if (( $# < 2 )); then
                echo "--qt-full-path requires a path" >&2
                exit 2
            fi
            if [[ -z "$2" ]]; then
                echo "--qt-full-path requires a non-empty path" >&2
                exit 2
            fi
            set_qt_full_path "$2"
            shift 2
            ;;
        --qt-full-path=*)
            qt_full_path="${1#*=}"
            if [[ -z "${qt_full_path}" ]]; then
                echo "--qt-full-path requires a non-empty path" >&2
                exit 2
            fi
            set_qt_full_path "${qt_full_path}"
            shift
            ;;
        --qt-version)
            if (( $# < 2 )); then
                echo "--qt-version requires a version" >&2
                exit 2
            fi
            if [[ -z "$2" ]]; then
                echo "--qt-version requires a non-empty version" >&2
                exit 2
            fi
            QT_VERSION="$2"
            shift 2
            ;;
        --qt-version=*)
            QT_VERSION="${1#*=}"
            if [[ -z "${QT_VERSION}" ]]; then
                echo "--qt-version requires a non-empty version" >&2
                exit 2
            fi
            shift
            ;;
        --)
            shift
            break
            ;;
        -*)
            echo "Unknown option: $1" >&2
            echo "Run $(basename "$0") --help for usage." >&2
            exit 2
            ;;
        *)
            set_qt_full_path "$1"
            shift
            ;;
    esac
done

if (( $# > 0 )); then
    if (( $# > 1 )); then
        echo "Only one full Qt path may be specified" >&2
        exit 2
    fi
    set_qt_full_path "$1"
fi

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
    elif [[ -n "${QT_ROOT_ARG}" ]]; then
        printf '%s/%s/macos\n' "${QT_ROOT_ARG%/}" "${QT_VERSION}"
    elif [[ -n "${QT_ROOT_DIR:-}" ]]; then
        printf '%s\n' "${QT_ROOT_DIR}"
    elif command -v qtpaths6 >/dev/null 2>&1; then
        qtpaths6 --query QT_INSTALL_PREFIX
    elif command -v qtpaths >/dev/null 2>&1; then
        qtpaths --query QT_INSTALL_PREFIX
    else
        echo "Unable to find Qt 6; pass --qt-prefix, --qt-full-path, or set QT_ROOT_DIR" >&2
        return 1
    fi
}

QT_PREFIX="$(find_qt_prefix "${QT_FULL_PATH_ARG}")"
MACDEPLOYQT="${QT_PREFIX}/bin/macdeployqt"
if [[ ! -x "${MACDEPLOYQT}" ]]; then
    echo "Unable to find macdeployqt at ${MACDEPLOYQT}" >&2
    exit 1
fi
QTPATHS="${QT_PREFIX}/bin/qtpaths"
if [[ ! -x "${QTPATHS}" ]]; then
    QTPATHS="${QT_PREFIX}/bin/qtpaths6"
fi
if [[ ! -x "${QTPATHS}" || "$("${QTPATHS}" --qt-version)" != "${QT_VERSION}" ]]; then
    echo "macOS release packaging requires the official Qt ${QT_VERSION} distribution" >&2
    exit 1
fi

QT_CORE="${QT_PREFIX}/lib/QtCore.framework/Versions/A/QtCore"
QT_ARCHITECTURES="$(lipo -archs "${QT_CORE}")"
if [[ " ${QT_ARCHITECTURES} " != *" arm64 "* ||
      " ${QT_ARCHITECTURES} " != *" x86_64 "* ]]; then
    echo "Qt ${QT_VERSION} must contain both arm64 and x86_64 architectures" >&2
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
