#!/usr/bin/env bash

# Dynaserve installer script
# Fetches and installs the Dynaserve CLI

REPO="Tydewest/CLI"

get_latest_release() {
  curl --silent "https://api.github.com/repos/$REPO/releases/latest" | grep '"tag_name":' | sed -E 's/.*"([^"]+)".*/\1/'
}

release_url() {
  echo "https://github.com/$REPO/releases"
}

download_release() {
  local version="$1"
  local tmpdir="$2"

  local filename="dynaserve"
  local download_file="$tmpdir/$filename"
  local archive_url="$(release_url)/download/$version/$filename"

  info 'Downloading' "Dynaserve $version"
  curl --progress-bar --show-error --location --fail "$archive_url" --output "$download_file" --write-out "$download_file"
}

usage() {
    cat >&2 <<END_USAGE
dynaserve-install: The installer for Dynaserve

USAGE:
    install.sh [FLAGS] [OPTIONS]

FLAGS:
    -h, --help                  Prints help information

OPTIONS:
        --version <version>     Install a specific release version (e.g., 1.0.0)
END_USAGE
}

info() {
  local action="$1"
  local details="$2"
  command printf '\033[1;32m%12s\033[0m %s\n' "$action" "$details" 1>&2
}

error() {
  command printf '\033[1;31mError\033[0m: %s\n\n' "$1" 1>&2
}

warning() {
  command printf '\033[1;33mWarning\033[0m: %s\n\n' "$1" 1>&2
}

eprintf() {
  command printf '%s\n' "$1" 1>&2
}

check_dependencies() {
  if ! command -v curl &> /dev/null; then
    error "curl is required but not installed. Please install curl and try again."
    exit 1
  fi

  if ! command -v shasum &> /dev/null && ! command -v sha256sum &> /dev/null; then
    warning "shasum/sha256sum not found. Skipping checksum verification."
    return 1
  fi
  return 0
}

verify_checksum() {
  local file="$1"
  local expected_sha="$2"
  
  if [ -z "$expected_sha" ]; then
    return 0
  fi

  local actual_sha
  if command -v shasum &> /dev/null; then
    actual_sha=$(shasum -a 256 "$file" | awk '{print $1}')
  elif command -v sha256sum &> /dev/null; then
    actual_sha=$(sha256sum "$file" | awk '{print $1}')
  else
    return 0
  fi

  if [ "$actual_sha" != "$expected_sha" ]; then
    error "Checksum mismatch! Expected: $expected_sha, Got: $actual_sha"
    return 1
  fi

  info 'Verified' "checksum matches"
  return 0
}

install_binary() {
  local binary_path="$1"
  local install_location="/usr/local/bin/dynaserve"

  info 'Installing' "dynaserve to $install_location"
  
  if [ -w "/usr/local/bin" ]; then
    mv "$binary_path" "$install_location"
  else
    sudo mv "$binary_path" "$install_location"
  fi

  if [ "$?" != 0 ]; then
    error "Failed to install dynaserve to $install_location"
    return 1
  fi

  chmod +x "$install_location"
  
  info 'Installed' "dynaserve successfully!"
  info 'Run' "dynaserve --help to get started"
  return 0
}

install_version() {
  local version="$1"
  local expected_checksum="$2"

  check_dependencies
  local can_verify=$?

  case "$version" in
    latest)
      local latest_version="$(get_latest_release)"
      if [ -z "$latest_version" ]; then
        error "Could not determine the latest version. Please check your internet connection."
        exit 1
      fi
      info 'Installing' "latest version of Dynaserve ($latest_version)"
      version="$latest_version"
      ;;
    *)
      info 'Installing' "Dynaserve version $version"
      ;;
  esac

  # Create temporary directory
  local tmpdir="$(mktemp -d)"
  
  # Download the release
  local download_file
  download_file="$(download_release "$version" "$tmpdir")"
  exit_status="$?"
  
  if [ "$exit_status" != 0 ] || [ ! -f "$download_file" ]; then
    error "Could not download Dynaserve version '$version'. See $(release_url) for available releases"
    rm -rf "$tmpdir"
    exit 1
  fi

  # Verify checksum if possible and provided
  if [ $can_verify -eq 0 ] && [ -n "$expected_checksum" ]; then
    if ! verify_checksum "$download_file" "$expected_checksum"; then
      rm -rf "$tmpdir"
      exit 1
    fi
  fi

  # Install the binary
  chmod +x "$download_file"
  if install_binary "$download_file"; then
    rm -rf "$tmpdir"
    return 0
  else
    rm -rf "$tmpdir"
    return 1
  fi
}

# return if sourced (for testing)
return 0 2>/dev/null

# Default to installing the latest version
version_to_install="latest"
expected_checksum=""

# Parse command line options
while [ $# -gt 0 ]
do
  arg="$1"

  case "$arg" in
    -h|--help)
      usage
      exit 0
      ;;
    --version)
      shift
      version_to_install="$1"
      shift
      ;;
    --checksum)
      shift
      expected_checksum="$1"
      shift
      ;;
    *)
      error "unknown option: '$arg'"
      usage
      exit 1
      ;;
  esac
done

install_version "$version_to_install" "$expected_checksum"