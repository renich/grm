#!/usr/bin/bash
set -euo pipefail
IFS=$'\n\t'

# Release preparation and tagging automation script for grm
# Enforces SemVer, Keep a Changelog, and GPG signed tags

usage() {
  echo "Usage: $0 <new-version>"
  echo "Example: $0 0.8.2"
  exit 1
}

main() {
  local target_version="${1:-}"

  if [[ -z "${target_version}" ]]; then
    usage
  fi

  if [[ ! "${target_version}" =~ ^[0-9]+\.[0-9]+\.[0-9]+$ ]]; then
    echo "Error: Version must follow Semantic Versioning (X.Y.Z)." >&2
    exit 1
  fi

  echo "==> Verifying repository cleanliness..."
  if ! git diff-index --quiet HEAD --; then
    echo "Error: Working directory is dirty. Commit or stash changes first." >&2
    exit 1
  fi

  echo "==> Running full test suite and quality gates..."
  make check

  echo "==> Updating version strings..."
  # Update CMakeLists.txt
  sed -i -E "s/^project\(grm VERSION [0-9]+\.[0-9]+\.[0-9]+/project(grm VERSION ${target_version}/" CMakeLists.txt

  # Update docs/conf.py if present
  if [[ -f docs/conf.py ]]; then
    sed -i -E "s/^release = .*/release = '${target_version}'/" docs/conf.py
  fi

  # Update packaging spec if present
  local spec_file
  for spec_file in packaging/*.spec; do
    if [[ -f "${spec_file}" ]]; then
      sed -i -E "s/^Version:\s+.*/Version:        ${target_version}/" "${spec_file}"
    fi
  done

  echo "==> Updating CHANGELOG.rst..."
  local release_date
  release_date="$(date +%Y-%m-%d)"

  if grep -q "\[Unreleased\]" CHANGELOG.rst; then
    sed -i "/\[Unreleased\]/a \\
\\
[${target_version}] - ${release_date}\\
====================" CHANGELOG.rst
  fi

  echo "==> Formatting code and verifying docs..."
  make format
  make doc-check

  echo "==> Creating release commit..."
  git add CMakeLists.txt docs/conf.py CHANGELOG.rst packaging/ 2>/dev/null || true
  git commit -m "chore(release): bump version to ${target_version}"

  echo "==> Creating GPG-signed tag v${target_version}..."
  git tag -s "v${target_version}" -m "Release v${target_version}"

  echo ""
  echo "Release v${target_version} prepared and tagged successfully."
  echo "To publish, execute: git push origin master --tags"
}

main "$@"
