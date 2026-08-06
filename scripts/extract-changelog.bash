#!/usr/bin/bash
set -euo pipefail
IFS=$'\n\t'

# extract-changelog.bash — Extract release notes for a version from CHANGELOG.rst
# Usage: ./scripts/extract-changelog.bash [version]

readonly VERSION="${1:-0.5.0}"
readonly CLEAN_VER="${VERSION#v}"
readonly CHANGELOG_FILE="CHANGELOG.rst"

if [[ ! -f "$CHANGELOG_FILE" ]]; then
  echo "Error: CHANGELOG.rst not found." >&2
  exit 1
fi

python3 -c "
import sys, re

version = sys.argv[1]
with open('CHANGELOG.rst', 'r') as f:
    lines = f.readlines()

capturing = False
notes = []

pattern = re.compile(rf'^\[{re.escape(version)}\]')

for line in lines:
    if pattern.search(line):
        capturing = True
        notes.append(line)
        continue
    if capturing:
        if re.match(r'^\[\d+\.\d+\.\d+\]', line) or line.startswith('[Unreleased]'):
            break
        notes.append(line)

output = ''.join(notes).strip()
if output:
    print(output)
else:
    print(f'Release {version} for grm (Group & Telegram Manager CLI).')
" "$CLEAN_VER"
