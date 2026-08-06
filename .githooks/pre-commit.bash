#!/usr/bin/bash
set -euo pipefail
IFS=$'\n\t'

# Pre-commit Hook for grm (Group & Telegram Manager CLI)
# Enforces crstlint, make doc-check, make format, make check (CTest), and make lint

readonly Red='\033[0;31m'
readonly Green='\033[0;32m'
readonly Cyan='\033[0;36m'
readonly Reset='\033[0m'

log_stage() {
  printf '%b==> %s%b\n' "$Cyan" "$1" "$Reset"
}

log_pass() {
  printf '%b[PASS] %s%b\n' "$Green" "$1" "$Reset"
}

log_fail() {
  printf '%b[FAIL] %s%b\n' "$Red" "$1" "$Reset"
}

log_stage "1/5 Running crstlint reStructuredText formatting check..."
if command -v crstlint >/dev/null 2>&1; then
  crstlint -fr . || { log_fail "crstlint checks failed. Run 'crstlint -fr .' to fix issues."; exit 1; }
elif [[ -x "/home/renich/.local/bin/crstlint" ]]; then
  /home/renich/.local/bin/crstlint -fr . || { log_fail "crstlint checks failed."; exit 1; }
else
  printf '%s\n' "[WARN] crstlint not found in PATH, skipping."
fi
log_pass "crstlint check passed."

log_stage "2/5 Running documentation syntax check (make doc-check)..."
make doc-check || { log_fail "make doc-check failed."; exit 1; }
log_pass "doc-check passed."

log_stage "3/5 Checking C++ code formatting (make format)..."
make format || { log_fail "make format failed."; exit 1; }
log_pass "code format passed."

log_stage "4/5 Building release binary & running CTest suite (make check)..."
make check || { log_fail "make check test suite failed."; exit 1; }
log_pass "CTest suite passed."

log_stage "5/5 Running static analysis & linters (make lint)..."
make lint || { log_fail "make lint static analysis failed."; exit 1; }
log_pass "Static analysis passed."

printf '%b[SUCCESS] All pre-commit quality checks passed! Proceeding with commit.%b\n' "$Green" "$Reset"
