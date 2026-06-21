#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
source "$SCRIPT_DIR/config"
source "$SCRIPT_DIR/lib/common.sh"

OUTPUT_DIR="$SCRIPT_DIR/output"
BUILD_DIR="$SCRIPT_DIR/.appimage-build"
APPDIR="$SCRIPT_DIR/.AppDir"
NCPUS="$(packaging_jobs)"

usage()
{
    echo "Usage: $0 [--version x.y.z] [--qt /path/to/Qt]"
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        --version)
            APP_VERSION="$2"
            shift 2
            ;;
        --qt)
            if [ -x "$2/bin/qmake6" ] || [ -x "$2/bin/qmake" ]; then
                export CMAKE_PREFIX_PATH="$2"
                export PATH="$2/bin:$PATH"
            elif [ -x "$2/qmake6" ] || [ -x "$2/qmake" ]; then
                export CMAKE_PREFIX_PATH="$(dirname "$2")"
                export PATH="$2:$PATH"
            else
                export CMAKE_PREFIX_PATH="$2"
            fi
            shift 2
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        *)
            echo "Unknown option: $1" >&2
            usage
            exit 1
            ;;
    esac
done

require_commands cmake make linuxdeploy linuxdeploy-plugin-qt linuxdeploy-plugin-appimage

rm -rf "$BUILD_DIR" "$APPDIR"
mkdir -p "$BUILD_DIR" "$APPDIR" "$OUTPUT_DIR"

echo "================================"
echo "Building Huggle AppImage"
echo "================================"
echo "Version: $APP_VERSION"
echo "CPUs: $NCPUS"
echo

"$SCRIPT_DIR/prepare-source.sh"

cmake -S "$PROJECT_ROOT/src" -B "$BUILD_DIR" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/usr \
    -DCMAKE_INSTALL_LIBDIR=lib \
    -DQT6_BUILD=true \
    -DWEB_ENGINE=true \
    -DHUGGLE_EXT=true \
    -DYAML_CPP_INSTALL=false
cmake --build "$BUILD_DIR" --parallel "$NCPUS"
DESTDIR="$APPDIR" cmake --install "$BUILD_DIR"

install -Dm644 "$SCRIPT_DIR/data/huggle.desktop" \
    "$APPDIR/usr/share/applications/huggle.desktop"
install -Dm644 "$SCRIPT_DIR/data/org.wikimedia.Huggle.metainfo.xml" \
    "$APPDIR/usr/share/metainfo/org.wikimedia.Huggle.metainfo.xml"
install -Dm644 "$PROJECT_ROOT/src/huggle_res/Resources/huggle3_newlogo.png" \
    "$APPDIR/usr/share/icons/hicolor/256x256/apps/huggle.png"

export QMAKE="${QMAKE:-$(command -v qmake6 || command -v qmake || true)}"
if [ -z "$QMAKE" ]; then
    echo "Error: qmake6 or qmake is required by linuxdeploy-plugin-qt." >&2
    exit 1
fi

rm -f "$PROJECT_ROOT"/*.AppImage
(
    cd "$PROJECT_ROOT"
    linuxdeploy \
        --appdir "$APPDIR" \
        --desktop-file "$APPDIR/usr/share/applications/huggle.desktop" \
        --icon-file "$APPDIR/usr/share/icons/hicolor/256x256/apps/huggle.png" \
        --plugin qt \
        --output appimage
)

mapfile -t APPIMAGES < <(
    find "$PROJECT_ROOT" -maxdepth 1 -type f -name '*.AppImage' -print | sort
)
if [ ${#APPIMAGES[@]} -eq 0 ]; then
    echo "Error: linuxdeploy did not produce an AppImage." >&2
    exit 1
fi

ARCH="$(uname -m)"
OUTPUT_PATH="$OUTPUT_DIR/huggle-${APP_VERSION}-${ARCH}.AppImage"
mv -f "${APPIMAGES[0]}" "$OUTPUT_PATH"
echo
echo "AppImage: $OUTPUT_PATH"
