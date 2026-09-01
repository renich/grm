# AGENTS Instructions: grm

## 1. Operating Directives
- **Partnership**: Equal engineering partner. Truthful, autonomous, zero fluff. Challenge flawed assumptions.
- **Verification**: Never assume; always verify against filesystem, specs, and compiler.
- **Brevity**: **Never be overly explicit.** Keep reasoning, responses, and documentation dense, direct, and succinct.
- **Code of Honor**: Strictly uphold [CODE_OF_HONOR.rst](CODE_OF_HONOR.rst).

## 2. Essential Commands
```bash
make setup              # Install dependencies, git hooks, and run all checks
make build              # Build development binary (build/grm)
make release            # Build stripped release binary
make static             # Build static binary via Podman
make test               # Run CTest test suite
make lint               # Run Clang-Tidy / static analyzers
make doc-check          # Lint RST docs with crstlint
make check              # Run test + doc-check
make asan               # Run AddressSanitizer & UBSan test suite
make tsan               # Run ThreadSanitizer test suite
make containers-build   # Build rootless Podman image
```

## 3. Engineering Standards
- **Modern C++23**: Enforces C++23 standards, explicit types, RAII, `std::expected` error handling, and zero warnings (`-Wall -Wextra -Wpedantic -Werror`).
- **Memory Safety & Concurrency**: ASan/UBSan and TSan verification. Zero memory leaks and strict thread safety.
- **Line Length**: Max 132 chars (`Layout/LineLength: MaxLength: 132` in doc/code standards).
- **Containers**: Podman only (rootless mode) with systemd Quadlets. Docker prohibited.
- **Scripts**: Mandatory `.bash` extension, `#!/usr/bin/bash`, `set -euo pipefail`, `IFS=$'\n\t'`.
- **Clean Workspace**: Experiments belong in `scratch/` (git-ignored). Never commit binaries or temp files.

## 4. Documentation & Specifications
- **Format**: Valid reStructuredText (`.rst`) validated via `crstlint`.
- **Architecture**: Multi-tier in `docs/` (`business/`, `functional/` with `[FUNC-xxx]`, `technical/` with `[TECH-xxx]`, `adrs/`, `project/`, `user/`).
- **RST Syntax**: 3-space indentation, blank line before nested lists, no `:caption:` on code blocks.

## 5. Release Protocol (MANDATORY)
1. Run `make check` (tests, linters, `crstlint` must pass 100%).
2. Align version in `CMakeLists.txt`, `packaging/*.spec`, `docs/conf.py`.
3. Update `CHANGELOG.rst` with `.. rubric::` section headings under new `[X.Y.Z] - YYYY-MM-DD`.
4. Commit and create GPG-signed tag: `git tag -s vX.Y.Z -m "Release vX.Y.Z"`.
5. Push to GitLab: `git push origin master --tags`.

## 6. Repository & Journaling
- **VCS**: GitLab only. Branch `master`, object format `sha256`. Conventional Commits enforced.
- **Journaling**: Use `ajourn` to persist architectural records and state in `.agents/journal/`.
