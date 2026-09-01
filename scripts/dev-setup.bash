#!/usr/bin/bash
set -euo pipefail
IFS=$'\n\t'

# Development environment setup script for grm
# Validates host tools, configures git hooks, and compiles dev dependencies

main() {
  local required_tools=("clang++" "cmake" "ninja" "make" "git")
  local missing_tools=()

  echo "==> Verifying host prerequisites..."
  for tool in "${required_tools[@]}"; do
    if ! command -v "${tool}" >/dev/null 2>&1; then
      missing_tools+=("${tool}")
    fi
  done

  if [[ ${#missing_tools[@]} -gt 0 ]]; then
    echo "Error: Missing required tools: ${missing_tools[*]}" >&2
    exit 1
  fi

  echo "==> Configuring Git hooks..."
  mkdir -p .githooks
  chmod +x .githooks/* 2>/dev/null || true
  git config core.hooksPath .githooks || true

  echo "==> Running initial verification suite..."
  make format
  make check

  echo "==> Workspace setup complete and fully verified."
}

main "$@"
