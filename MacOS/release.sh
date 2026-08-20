#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd -- "${SCRIPT_DIR}/.." && pwd)"
SOURCE_DIR="${ROOT_DIR}/src"
DEFAULT_QT_VERSION="6.9.2"
QT_VERSION="${HUGGLE_QT_VERSION:-${DEFAULT_QT_VERSION}}"
QT_ROOT_ARG=""
QT_FULL_PATH_ARG=""
PACKAGE_MODE="universal"
PACKAGE_MODE_SET=false

usage()
{
    cat <<EOF
Usage: $(basename "$0") [options] [QT_FULL_PATH]

Build, test, and package macOS Huggle release DMGs.

Options:
  -h, --help                 Show this help text and exit.
      --universal            Build the universal arm64/x86_64 DMG. This is
                             the default.
      --intel                Build an Intel x86_64-only DMG.
      --arm                  Build an Apple Silicon arm64-only DMG.
      --all                  Build Intel, Apple Silicon, and universal DMGs.
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
                                  release/macos-<variant>. With --all and an
                                  explicit HUGGLE_BUILD_DIR, each variant uses
                                  a subdirectory below HUGGLE_BUILD_DIR.
  HUGGLE_OUTPUT_DIR               DMG output directory, default
                                  release/artifacts.
  CMAKE_BUILD_PARALLEL_LEVEL      Number of parallel build jobs, default
                                  the host CPU count.

Examples:
  ./MacOS/release.sh --qt-prefix ~/Qt --qt-version 6.9.2
  ./MacOS/release.sh --intel --qt-prefix ~/Qt --qt-version 6.9.2
  ./MacOS/release.sh --all --qt-full-path /path/to/Qt/6.9.2/macos
  ./MacOS/release.sh --qt-full-path /path/to/Qt/6.9.2/macos
  ./MacOS/release.sh /path/to/Qt/6.9.2/macos
EOF
}

set_package_mode()
{
    if [[ "${PACKAGE_MODE_SET}" == true ]]; then
        echo "Only one architecture mode may be specified" >&2
        exit 2
    fi
    PACKAGE_MODE="$1"
    PACKAGE_MODE_SET=true
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
        --universal)
            set_package_mode "universal"
            shift
            ;;
        --intel)
            set_package_mode "intel"
            shift
            ;;
        --arm)
            set_package_mode "arm"
            shift
            ;;
        --all)
            set_package_mode "all"
            shift
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

OUTPUT_DIR="${HUGGLE_OUTPUT_DIR:-${ROOT_DIR}/release/artifacts}"
PARALLEL="${CMAKE_BUILD_PARALLEL_LEVEL:-$(sysctl -n hw.ncpu)}"

thin_macho_file()
{
    local candidate="$1"
    local architecture="$2"
    local architectures
    local tmp

    if ! file -b "${candidate}" | grep -q '^Mach-O'; then
        return 0
    fi

    architectures="$(lipo -archs "${candidate}")"
    if [[ " ${architectures} " != *" ${architecture} "* ]]; then
        echo "Mach-O file does not contain ${architecture}: ${candidate}" >&2
        exit 1
    fi
    if [[ "${architectures}" == "${architecture}" ]]; then
        return 0
    fi

    tmp="${candidate}.thin-${architecture}"
    rm -f "${tmp}"
    lipo "${candidate}" -thin "${architecture}" -output "${tmp}"
    chmod "$(stat -f %Lp "${candidate}")" "${tmp}"
    mv "${tmp}" "${candidate}"
}

thin_app_bundle()
{
    local app_bundle="$1"
    local architecture="$2"
    local candidate

    while IFS= read -r -d '' candidate; do
        thin_macho_file "${candidate}" "${architecture}"
    done < <(find "${app_bundle}/Contents" -type f -print0)
}

variant_build_root()
{
    local variant="$1"

    if [[ -n "${HUGGLE_BUILD_DIR:-}" ]]; then
        if [[ "${PACKAGE_MODE}" == "all" ]]; then
            printf '%s/%s\n' "${HUGGLE_BUILD_DIR%/}" "${variant}"
        else
            printf '%s\n' "${HUGGLE_BUILD_DIR}"
        fi
    else
        printf '%s/release/macos-%s\n' "${ROOT_DIR}" "${variant}"
    fi
}

build_package_variant()
{
    local variant="$1"
    local cmake_architectures="$2"
    local verify_architecture="$3"
    local build_root
    local build_dir
    local stage_dir
    local app_bundle
    local frameworks_dir
    local plugins_dir
    local version
    local output_dir_abs
    local dmg_path
    local dmg_created
    local attempt
    local extension

    echo "Building ${variant} macOS package"

    build_root="$(variant_build_root "${variant}")"
    build_dir="${build_root}/build"
    stage_dir="${build_root}/stage"

    cmake -S "${SOURCE_DIR}" -B "${build_dir}" \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_BINDIR:PATH=bin \
        -DCMAKE_INSTALL_DATADIR:PATH=share \
        -DCMAKE_INSTALL_LIBDIR:PATH=lib \
        -DCMAKE_INSTALL_PREFIX:PATH=/usr/local \
        -DCMAKE_OSX_ARCHITECTURES="${cmake_architectures}" \
        -DCMAKE_PREFIX_PATH="${QT_PREFIX}" \
        -DQT5_BUILD=OFF \
        -DQT6_BUILD=ON \
        -DWEB_ENGINE=ON \
        -DHUGGLE_EXT=ON \
        -DHUGGLE_TEST=ON \
        -DCMAKE_DISABLE_FIND_PACKAGE_YAML-CPP=ON \
        -DYAML_BUILD_SHARED_LIBS=OFF

    cmake --build "${build_dir}" --parallel "${PARALLEL}"
    ctest --test-dir "${build_dir}" --output-on-failure

    cmake -E remove_directory "${stage_dir}"
    cmake --install "${build_dir}" --prefix "${stage_dir}"

    app_bundle="${stage_dir}/huggle.app"
    frameworks_dir="${app_bundle}/Contents/Frameworks"
    plugins_dir="${app_bundle}/Contents/PlugIns"

    if [[ ! -d "${app_bundle}" ]]; then
        echo "CMake did not install ${app_bundle}" >&2
        exit 1
    fi

    mkdir -p "${frameworks_dir}" "${plugins_dir}"
    shopt -s nullglob

    PROJECT_LIBRARIES=("${stage_dir}/lib/"*.dylib)
    if (( ${#PROJECT_LIBRARIES[@]} == 0 )); then
        echo "No Huggle libraries were installed in ${stage_dir}/lib" >&2
        exit 1
    fi
    cp -R -p "${PROJECT_LIBRARIES[@]}" "${frameworks_dir}/"

    COMPILED_EXTENSIONS=("${stage_dir}/share/huggle/extensions/"*.dylib)
    SCRIPT_EXTENSIONS=("${stage_dir}/share/huggle/extensions/"*.js)
    if (( ${#COMPILED_EXTENSIONS[@]} == 0 || ${#SCRIPT_EXTENSIONS[@]} == 0 )); then
        echo "Huggle extensions were not installed completely" >&2
        exit 1
    fi
    cp -R -p "${COMPILED_EXTENSIONS[@]}" "${plugins_dir}/"

    DEPLOY_ARGS=("${app_bundle}" "-always-overwrite"
                 "-libpath=${frameworks_dir}" "-libpath=${QT_PREFIX}/lib")
    for extension in "${plugins_dir}/"*.dylib; do
        DEPLOY_ARGS+=("-executable=${extension}")
    done
    "${MACDEPLOYQT}" "${DEPLOY_ARGS[@]}"

    # Huggle does not use NMEA GPS positioning, and this plugin adds a
    # QtSerialPort runtime dependency that is not needed by the application.
    rm -f "${plugins_dir}/position/libqtposition_nmea.dylib"

    cp -R -p "${SCRIPT_EXTENSIONS[@]}" "${plugins_dir}/"
    if [[ "${verify_architecture}" != "universal" ]]; then
        thin_app_bundle "${app_bundle}" "${verify_architecture}"
    fi
    codesign --force --deep --sign - "${app_bundle}"
    "${SCRIPT_DIR}/verify-app.sh" --arch "${verify_architecture}" "${app_bundle}"

    version="$(plutil -extract CFBundleShortVersionString raw "${app_bundle}/Contents/Info.plist")"
    mkdir -p "${OUTPUT_DIR}"
    output_dir_abs="$(cd "${OUTPUT_DIR}" && pwd -P)"
    dmg_path="${output_dir_abs}/huggle_${version}_${variant}.dmg"
    rm -f "${dmg_path}"
    dmg_created=false
    for attempt in 1 2 3; do
        if hdiutil create -volname "Huggle ${version}" -srcfolder "${app_bundle}" \
            -format UDZO -ov "${dmg_path}"; then
            dmg_created=true
            break
        fi
        if (( attempt < 3 )); then
            echo "DMG creation failed; retrying (${attempt}/3)" >&2
            rm -f "${dmg_path}"
            sleep 5
        fi
    done
    if [[ "${dmg_created}" != true ]]; then
        echo "Unable to create ${dmg_path} after 3 attempts" >&2
        exit 1
    fi

    printf 'Package path: %s\n' "${dmg_path}"
}

case "${PACKAGE_MODE}" in
    universal)
        build_package_variant "universal" "arm64;x86_64" "universal"
        ;;
    intel)
        build_package_variant "intel" "x86_64" "x86_64"
        ;;
    arm)
        build_package_variant "arm" "arm64" "arm64"
        ;;
    all)
        build_package_variant "intel" "x86_64" "x86_64"
        build_package_variant "arm" "arm64" "arm64"
        build_package_variant "universal" "arm64;x86_64" "universal"
        ;;
    *)
        echo "Unknown package mode: ${PACKAGE_MODE}" >&2
        exit 2
        ;;
esac
