#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
source "$SCRIPT_DIR/config"
source "$SCRIPT_DIR/lib/common.sh"

TARGET=""
RELEASE="1"
NCPUS="$(packaging_jobs)"

usage()
{
    echo "Usage: $0 [--version x.y.z]"
    echo "Builds for the Fedora or Rocky Linux system detected from /etc/os-release."
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

require_commands rpm rpmbuild cmake make tar gzip

if [ -r /etc/os-release ]; then
    . /etc/os-release
    if [ -z "$TARGET" ]; then
        case "${ID:-}" in
            fedora|rocky)
                TARGET="$ID"
                ;;
            *)
                echo "Error: package-rpm.sh must run on Fedora or Rocky Linux; detected ${ID:-unknown}." >&2
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
    rpm-build
    gcc-c++
    cmake
    make
    pkgconf-pkg-config
    qt6-qtbase-devel
    qt6-qtdeclarative-devel
    qt6-qtwebengine-devel
    qt6-qtmultimedia-devel
    yaml-cpp-devel
)
MISSING=()

for package_name in "${BUILD_DEPENDENCIES[@]}"; do
    if ! rpm -q "$package_name" >/dev/null 2>&1; then
        MISSING+=("$package_name")
    fi
done

if [ ${#MISSING[@]} -gt 0 ]; then
    echo "Missing build dependencies:"
    printf '  %s\n' "${MISSING[@]}"
    echo
    if [ "$TARGET" = "rocky" ]; then
        echo "Rocky Linux may require CRB and EPEL before installing Qt WebEngine:"
        echo "  sudo dnf config-manager --set-enabled crb"
        echo "  sudo dnf install epel-release"
    fi
    echo "Install dependencies with:"
    echo "  sudo dnf install ${MISSING[*]}"
    exit 1
fi

WORK_DIR="$SCRIPT_DIR/.rpm-build/$TARGET"
RPM_TOPDIR="$WORK_DIR/rpmbuild"
OUTPUT_DIR="$SCRIPT_DIR/output"
SOURCE_DIR_NAME="${APP_NAME}-${APP_VERSION}"
SOURCE_TARBALL="$RPM_TOPDIR/SOURCES/${SOURCE_DIR_NAME}.tar.gz"
SPEC_PATH="$RPM_TOPDIR/SPECS/${APP_NAME}.spec"

rm -rf "$WORK_DIR"
mkdir -p "$RPM_TOPDIR"/{BUILD,BUILDROOT,RPMS,SOURCES,SPECS,SRPMS} "$OUTPUT_DIR"

echo "================================"
echo "Building Huggle for ${TARGET^}"
echo "================================"
echo "Version: $APP_VERSION-$RELEASE"
echo "CPUs: $NCPUS"
echo
echo "Step 1: Preparing source archive..."
source_archive "$PROJECT_ROOT" "$SOURCE_DIR_NAME" "$SOURCE_TARBALL"

cat > "$SPEC_PATH" <<EOF
Name:           huggle
Version:        $APP_VERSION
Release:        $RELEASE%{?dist}
Summary:        $DESCRIPTION

License:        GPL-3.0-or-later
URL:            $APP_HOMEPAGE_URL
Source0:        %{name}-%{version}.tar.gz

BuildRequires:  gcc-c++
BuildRequires:  cmake
BuildRequires:  make
BuildRequires:  pkgconfig
BuildRequires:  qt6-qtbase-devel
BuildRequires:  qt6-qtdeclarative-devel
BuildRequires:  qt6-qtwebengine-devel
BuildRequires:  qt6-qtmultimedia-devel
BuildRequires:  yaml-cpp-devel

%description
Huggle is a fast diff browser for Wikipedia and other MediaWiki sites,
designed to help reviewers detect and revert vandalism.

%prep
%autosetup

%build
./packaging/prepare-source.sh
cmake -S src -B build \\
    -DCMAKE_BUILD_TYPE=Release \\
    -DCMAKE_INSTALL_PREFIX=%{_prefix} \\
    -DCMAKE_INSTALL_LIBDIR=%{_lib} \\
    -DQT6_BUILD=true \\
    -DWEB_ENGINE=true \\
    -DHUGGLE_EXT=true \\
    -DYAML_CPP_INSTALL=false
%make_build -C build

%install
rm -rf %{buildroot}
DESTDIR=%{buildroot} cmake --install build
install -Dm644 packaging/data/huggle.desktop %{buildroot}%{_datadir}/applications/huggle.desktop
install -Dm644 packaging/data/org.wikimedia.Huggle.metainfo.xml \\
    %{buildroot}%{_datadir}/metainfo/org.wikimedia.Huggle.metainfo.xml
install -Dm644 src/huggle_res/Resources/huggle3_newlogo.png \\
    %{buildroot}%{_datadir}/icons/hicolor/256x256/apps/huggle.png

%files
%license LICENSE
%doc README.md
%{_bindir}/huggle
%{_libdir}/libhuggle_*.so*
%{_libdir}/libirc*.so*
%{_datadir}/huggle/
%{_datadir}/applications/huggle.desktop
%{_datadir}/icons/hicolor/256x256/apps/huggle.png
%{_datadir}/metainfo/org.wikimedia.Huggle.metainfo.xml
%{_mandir}/man1/huggle.1*

%changelog
* $(date '+%a %b %d %Y') $MAINTAINER - $APP_VERSION-$RELEASE
- Automated ${TARGET^} package build
EOF

echo "Step 2: Building RPM and source RPM..."
rpmbuild --define "_topdir $RPM_TOPDIR" -ba "$SPEC_PATH"

find "$RPM_TOPDIR/RPMS" "$RPM_TOPDIR/SRPMS" -type f -name '*.rpm' \
    -exec cp -f {} "$OUTPUT_DIR/" \;

echo
echo "Package(s):"
find "$OUTPUT_DIR" -maxdepth 1 -type f -name "huggle-${APP_VERSION}-${RELEASE}*.rpm" -print | sort
