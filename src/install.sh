#!/bin/bash

# Dynaserve auto-installer & updater
# Usage: curl -sSL https://raw.githubusercontent.com/Tydewest/CLI/main/install.sh | bash

BINARY_NAME="dynaserve"
TARGET_DIR="/usr/local/bin"
TARGET_PATH="$TARGET_DIR/$BINARY_NAME"

echo "Installing/Updating Dynaserve CLI..."

# Get latest release version from GitHub
LATEST_VERSION=$(curl -s https://api.github.com/repos/Tydewest/CLI/releases/latest \
    | grep tag_name | head -n1 | cut -d'"' -f4)

if [ -z "$LATEST_VERSION" ]; then
    echo "Failed to detect the latest Dynaserve version."
    exit 1
fi

echo "Latest version: $LATEST_VERSION"

DOWNLOAD_URL="https://github.com/Tydewest/CLI/releases/download/$LATEST_VERSION/dynaserve"
CHECKSUM_URL="https://github.com/Tydewest/CLI/releases/download/$LATEST_VERSION/dynaserve.sha256"

# Download binary to temp file
TMP_FILE=$(mktemp)
echo "Downloading Dynaserve CLI v$LATEST_VERSION..."
curl -L -o "$TMP_FILE" "$DOWNLOAD_URL"

if [ ! -f "$TMP_FILE" ]; then
    echo "Download failed."
    exit 1
fi

# Download SHA256 checksum
TMP_CHECKSUM=$(mktemp)
curl -L -o "$TMP_CHECKSUM" "$CHECKSUM_URL"

if [ ! -f "$TMP_CHECKSUM" ]; then
    echo "Failed to download checksum."
    rm -f "$TMP_FILE"
    exit 1
fi

# Verify checksum
echo "Verifying checksum..."
DOWNLOADED_SUM=$(shasum -a 256 "$TMP_FILE" | awk '{print $1}')
EXPECTED_SUM=$(cat "$TMP_CHECKSUM" | awk '{print $1}')

if [ "$DOWNLOADED_SUM" != "$EXPECTED_SUM" ]; then
    echo "Checksum mismatch! Aborting installation."
    rm -f "$TMP_FILE" "$TMP_CHECKSUM"
    exit 1
fi

rm -f "$TMP_CHECKSUM"

# Backup existing binary if exists
if [ -f "$TARGET_PATH" ]; then
    CURRENT_VERSION=$($TARGET_PATH version 2>/dev/null | grep -Eo '[0-9]+\.[0-9]+\.[0-9]+')
    if [ "$CURRENT_VERSION" == "$LATEST_VERSION" ]; then
        echo "Dynaserve CLI is already up-to-date (v$LATEST_VERSION)."
        rm -f "$TMP_FILE"
        exit 0
    fi
    echo "Backing up existing $TARGET_PATH to ${TARGET_PATH}_backup"
    sudo mv "$TARGET_PATH" "${TARGET_PATH}_backup"
fi

# Move new binary to /usr/local/bin
sudo mv "$TMP_FILE" "$TARGET_PATH"
sudo chmod +x "$TARGET_PATH"

echo "Dynaserve CLI v$LATEST_VERSION installed/updated successfully!"
echo "Run 'dynaserve help' to get started."