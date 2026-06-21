#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
CORE_DIR="$PROJECT_ROOT/src/huggle_core"

if [ ! -f "$CORE_DIR/definitions_prod.hpp" ]; then
    echo "Error: missing $CORE_DIR/definitions_prod.hpp" >&2
    exit 1
fi

# This mirrors the preparation performed by ./configure and the existing
# platform packaging scripts.
if [ ! -f "$CORE_DIR/definitions.hpp" ]; then
    cp "$CORE_DIR/definitions_prod.hpp" "$CORE_DIR/definitions.hpp"
fi

(
    cd "$CORE_DIR"
    ./update.sh
)

if [ ! -f "$CORE_DIR/version.txt" ]; then
    echo "Error: failed to generate $CORE_DIR/version.txt" >&2
    exit 1
fi
