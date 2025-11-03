#!/usr/bin/env bash

# Dynaserve installer (static template)
BIN_URL="https://github.com/Tydewest/CLI/releases/download/LATEST/dynaserve"
EXPECTED_SHA256="INSERT_SHA256_HERE"

echo "Installing Dynaserve CLI..."

curl -L -o dynaserve "$BIN_URL"

# Verify checksum
DOWNLOADED_SHA256=$(shasum -a 256 dynaserve | awk '{print $1}')
if [ "$DOWNLOADED_SHA256" != "$EXPECTED_SHA256" ]; then
    echo "Checksum mismatch! Aborting installation."
    exit 1
fi

chmod +x dynaserve
mv dynaserve /usr/local/bin/dynaserve

echo "Dynaserve installed successfully!"