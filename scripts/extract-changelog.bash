#!/usr/bin/env bash
set -euo pipefail
IFS=$'\n\t'

# extract-changelog.bash — Extract and convert release notes from CHANGELOG.rst to Markdown
# Usage: ./scripts/extract-changelog.bash [version]

readonly VERSION="${1:-0.5.0}"
readonly CLEAN_VER="${VERSION#v}"
readonly CHANGELOG_FILE="CHANGELOG.rst"

if [[ ! -f "$CHANGELOG_FILE" ]]; then
  echo "Error: CHANGELOG.rst not found." >&2
  exit 1
fi

sed -n "/^\[${CLEAN_VER}\]/,/^\[[0-9]/p" "$CHANGELOG_FILE" \
  | sed '/^\[/d; /^=/d; s/^\.\. rubric:: \(.*\)/### \1/; s/``/`/g' \
  | sed '$d'
