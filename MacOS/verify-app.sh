#!/usr/bin/env bash

set -euo pipefail

if (( $# != 1 )); then
    echo "Usage: $0 <app-bundle>" >&2
    exit 1
fi

APP_BUNDLE="$1"
PLIST="${APP_BUNDLE}/Contents/Info.plist"

require_universal()
{
    local candidate="$1"
    case "$(lipo -archs "${candidate}")" in
        "arm64 x86_64"|"x86_64 arm64") ;;
        *)
            echo "Mach-O file is not universal arm64/x86_64: ${candidate}" >&2
            exit 1
            ;;
    esac
}

test -d "${APP_BUNDLE}"
test -x "${APP_BUNDLE}/Contents/MacOS/huggle"
plutil -lint "${PLIST}" >/dev/null

while IFS= read -r -d '' candidate; do
    if ! file -b "${candidate}" | grep -q '^Mach-O'; then
        continue
    fi
    require_universal "${candidate}"
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
done < <(find "${APP_BUNDLE}/Contents/MacOS" \
              "${APP_BUNDLE}/Contents/Frameworks" \
              "${APP_BUNDLE}/Contents/PlugIns" \
              -type f -print0)

codesign --verify --deep --strict "${APP_BUNDLE}"

printf 'Validated universal application %s\n' "${APP_BUNDLE}"
