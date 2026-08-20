#!/usr/bin/env bash

set -euo pipefail

ARCHITECTURE="universal"

usage()
{
    echo "Usage: $0 [--arch universal|arm64|x86_64] <app-bundle>" >&2
}

while (( $# > 0 )); do
    case "$1" in
        --arch)
            if (( $# < 2 )); then
                usage
                exit 2
            fi
            ARCHITECTURE="$2"
            shift 2
            ;;
        --arch=*)
            ARCHITECTURE="${1#*=}"
            if [[ -z "${ARCHITECTURE}" ]]; then
                usage
                exit 2
            fi
            shift
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        --)
            shift
            break
            ;;
        -*)
            usage
            exit 2
            ;;
        *)
            break
            ;;
    esac
done

if (( $# != 1 )); then
    usage
    exit 1
fi

case "${ARCHITECTURE}" in
    universal|arm64|x86_64) ;;
    *)
        usage
        exit 2
        ;;
esac

APP_BUNDLE="$1"
PLIST="${APP_BUNDLE}/Contents/Info.plist"

require_architecture()
{
    local candidate="$1"
    local architectures

    architectures="$(lipo -archs "${candidate}")"
    if [[ "${ARCHITECTURE}" == "universal" ]]; then
        case "${architectures}" in
            "arm64 x86_64"|"x86_64 arm64") ;;
            *)
                echo "Mach-O file is not universal arm64/x86_64: ${candidate}" >&2
                exit 1
                ;;
        esac
    elif [[ "${architectures}" != "${ARCHITECTURE}" ]]; then
        echo "Mach-O file is not ${ARCHITECTURE}-only: ${candidate}" >&2
        exit 1
    fi
}

test -d "${APP_BUNDLE}"
test -x "${APP_BUNDLE}/Contents/MacOS/huggle"
plutil -lint "${PLIST}" >/dev/null

while IFS= read -r -d '' candidate; do
    if ! file -b "${candidate}" | grep -q '^Mach-O'; then
        continue
    fi
    require_architecture "${candidate}"
    install_names="$(otool -D "${candidate}" | sed -E '/:$/d; /^[[:space:]]*$/d; s/^[[:space:]]+//')"
    while IFS= read -r dependency; do
        [[ "${dependency}" =~ ^[[:space:]] ]] || continue
        dependency="${dependency#"${dependency%%[![:space:]]*}"}"
        dependency="${dependency%% (*}"
        if grep -Fqx "${dependency}" <<< "${install_names}"; then
            continue
        fi
        case "${dependency}" in
            @rpath/*)
                target="${APP_BUNDLE}/Contents/Frameworks/${dependency#@rpath/}"
                ;;
            @loader_path/*)
                target="$(dirname "${candidate}")/${dependency#@loader_path/}"
                ;;
            @executable_path/*)
                target="${APP_BUNDLE}/Contents/MacOS/${dependency#@executable_path/}"
                ;;
            /System/Library/*|/usr/lib/*)
                continue
                ;;
            *)
                echo "Non-relocatable dependency in ${candidate}: ${dependency}" >&2
                exit 1
                ;;
        esac
        if [[ ! -e "${target}" ]]; then
            echo "Missing bundled dependency for ${candidate}: ${dependency}" >&2
            exit 1
        fi
    done < <(otool -L "${candidate}")
done < <(find "${APP_BUNDLE}/Contents" -type f -print0)

codesign --verify --deep --strict "${APP_BUNDLE}"

printf 'Validated %s application %s\n' "${ARCHITECTURE}" "${APP_BUNDLE}"
