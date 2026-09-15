#!/bin/bash
set -eu

SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)
PLUGIN_PATH="${SCRIPT_DIR}/install/plugin/usd/cgpValidatorPlugin/resources"

export PXR_PLUGINPATH_NAME="${PLUGIN_PATH}"

if [ "$#" -eq 0 ]; then
    echo "Usage: $0 <usd-file>"
    exit 1
fi

USD_FILE="$1"
if [ ! -f "$USD_FILE" ]; then
    echo "File not found: $USD_FILE"
    exit 1
fi

usdchecker "$USD_FILE" --includeKeywords commonKeyword
