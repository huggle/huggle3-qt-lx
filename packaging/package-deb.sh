#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
source "$SCRIPT_DIR/config"
source "$SCRIPT_DIR/lib/common.sh"

TARGET=""
NCPUS="$(packaging_jobs)"

usage()
{
    echo "Usage: $0 [--version x.y.z]"
    echo "Builds for the Debian or Ubuntu system detected from /etc/os-release."
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        --target)
            TARGET="$2"
            shift 2
            ;;
        --version)
            APP_VERSION="$2"
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

require_commands dpkg-buildpackage dpkg-query cmake make

if [ -r /etc/os-release ]; then
    . /etc/os-release
    if [ -z "$TARGET" ]; then
        case "${ID:-}" in
            debian|ubuntu)
                TARGET="$ID"
                ;;
            *)
                echo "Error: package-deb.sh must run on Debian or Ubuntu; detected ${ID:-unknown}." >&2
                exit 1
                ;;
        esac
    elif [[ "${ID:-}" != "$TARGET" ]]; then
        echo "Error: this script targets $TARGET, but the current system is ${ID:-unknown}." >&2
        exit 1
    fi
else
    echo "Error: unable to detect the target distribution because /etc/os-release is missing." >&2
    exit 1
fi

BUILD_DEPENDENCIES=(
    build-essential
    debhelper
    dpkg-dev
    cmake
    pkg-config
    qt6-base-dev
    qt6-declarative-dev
    qt6-webengine-dev
    qt6-multimedia-dev
    libyaml-cpp-dev
)
MISSING=()

for package_name in "${BUILD_DEPENDENCIES[@]}"; do
    if ! dpkg-query -W -f='${Status}' "$package_name" 2>/dev/null |
        grep -q "install ok installed"; then
        MISSING+=("$package_name")
    fi
done

if [ ${#MISSING[@]} -gt 0 ]; then
    echo "Missing build dependencies:"
    printf '  %s\n' "${MISSING[@]}"
    echo
    echo "Install them with:"
    echo "  sudo apt-get install ${MISSING[*]}"
    exit 1
fi

DISTRO_VERSION="${VERSION_ID:-unknown}"
DISTRO_TAG="${TARGET}${DISTRO_VERSION}"
DEB_VERSION="${APP_VERSION}-1~${DISTRO_TAG}"
CHANGELOG="$PROJECT_ROOT/debian/changelog"
CHANGELOG_BACKUP="$(mktemp)"
cp "$CHANGELOG" "$CHANGELOG_BACKUP"
trap 'cp "$CHANGELOG_BACKUP" "$CHANGELOG"; rm -f "$CHANGELOG_BACKUP"' EXIT

cat > "$CHANGELOG" <<EOF
huggle (${DEB_VERSION}) unstable; urgency=medium

  * Automated ${TARGET^} package build.

 -- ${MAINTAINER}  $(date -R)
EOF

echo "================================"
echo "Building Huggle for ${TARGET^}"
echo "================================"
echo "Version: $DEB_VERSION"
echo "CPUs: $NCPUS"
echo

cd "$PROJECT_ROOT"
DEB_BUILD_OPTIONS="parallel=$NCPUS" dpkg-buildpackage -b -us -uc

OUTPUT_DIR="$SCRIPT_DIR/output"
OUTPUT_PARENT="$(dirname "$PROJECT_ROOT")"
mkdir -p "$OUTPUT_DIR"

mapfile -t DEB_FILES < <(
    find "$OUTPUT_PARENT" "$PROJECT_ROOT" -maxdepth 1 -type f \
        \( -name "huggle_${DEB_VERSION}_*.deb" -o -name "huggle-dbgsym_${DEB_VERSION}_*.deb" \) \
        -print | sort -u
)

if [ ${#DEB_FILES[@]} -eq 0 ]; then
    echo "Error: dpkg-buildpackage completed without producing a .deb file." >&2
    exit 1
fi

for artifact in "${DEB_FILES[@]}"; do
    mv -f "$artifact" "$OUTPUT_DIR/"
done

echo
echo "Package(s):"
find "$OUTPUT_DIR" -maxdepth 1 -type f -name "*_${DEB_VERSION}_*.deb" -print | sort
