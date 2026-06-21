#!/usr/bin/env bash

packaging_jobs()
{
    nproc 2>/dev/null || getconf _NPROCESSORS_ONLN 2>/dev/null || echo 1
}

require_commands()
{
    local missing=()
    local command_name

    for command_name in "$@"; do
        if ! command -v "$command_name" >/dev/null 2>&1; then
            missing+=("$command_name")
        fi
    done

    if [ ${#missing[@]} -gt 0 ]; then
        echo "Error: missing required command(s): ${missing[*]}" >&2
        return 1
    fi
}

source_archive()
{
    local project_root="$1"
    local source_dir_name="$2"
    local output="$3"
    local staging

    staging="$(mktemp -d)"
    trap 'rm -rf "$staging"' RETURN
    mkdir -p "$staging/$source_dir_name"

    tar \
        --exclude-vcs \
        --exclude='./TuxManager' \
        --exclude='./release' \
        --exclude='./debug' \
        --exclude='./packaging/output' \
        --exclude='./packaging/.appimage-build' \
        --exclude='./packaging/.AppDir' \
        --exclude='./packaging/.rpm-build' \
        -cf - -C "$project_root" . |
        tar -xf - -C "$staging/$source_dir_name"

    tar -C "$staging" -czf "$output" "$source_dir_name"
    rm -rf "$staging"
    trap - RETURN
}
