# GNUmakefile for grm (Group & Telegram Manager CLI in C++23)
# Compliant with GNU Coding Standards & POSIX Conventions

SHELL := /usr/bin/bash
.SHELLFLAGS := -eu -o pipefail -c

.DEFAULT_GOAL := all

# Standard GNU Installation Directories
PREFIX ?= /usr/local
EXEC_PREFIX ?= $(PREFIX)
BINDIR ?= $(EXEC_PREFIX)/bin
DATAROOTDIR ?= $(PREFIX)/share
MANDIR ?= $(DATAROOTDIR)/man
MAN1DIR ?= $(MANDIR)/man1
BASH_COMPLETION_DIR ?= $(DATAROOTDIR)/bash-completion/completions

# User Installation Directories (~/.local/bin, ~/.local/share)
USER_BIN ?= $(HOME)/.local/bin
USER_MAN_DIR ?= $(HOME)/.local/share/man/man1
USER_BASH_COMPLETION_DIR ?= $(HOME)/.local/share/bash-completion/completions

# Build Toolchain & Directories
BUILD_DIR := build
CMAKE ?= cmake
CTEST ?= ctest
STRIP ?= strip
CLANG_FORMAT ?= clang-format
CLANG_TIDY ?= clang-tidy
SCAN_BUILD ?= scan-build

.PHONY: all build release clean distclean format lint analyze check man doc-check install install-user install-man install-user-man install-completions install-user-completions install-hooks hooks uninstall uninstall-user help

all: build

help:
	@echo "grm - GNUmakefile Target Summary"
	@echo ""
	@echo "Build Targets:"
	@echo "  all                  Build debug executable (default goal)"
	@echo "  build                Build unoptimized debug binary in $(BUILD_DIR)/"
	@echo "  release              Build optimized release binary in $(BUILD_DIR)/ and strip symbols"
	@echo ""
	@echo "Quality & Testing Targets:"
	@echo "  check                Run full CTest automated test suite"
	@echo "  format               Apply clang-format code formatting to src/ and include/"
	@echo "  lint                 Run clang-tidy static analysis on C++ source files"
	@echo "  analyze              Run Clang scan-build static analyzer"
	@echo "  doc-check            Verify rstcheck syntax for user guide and man page"
	@echo "  man                  Compile reStructuredText man page to $(BUILD_DIR)/grm.1"
	@echo "  install-hooks        Configure repository Git pre-push hook (.githooks/pre-push.bash)"
	@echo ""
	@echo "Installation Targets:"
	@echo "  install              Install release binary, man page, and completions to $(PREFIX)"
	@echo "  install-user         Install release binary, man page, and completions to $(HOME)/.local"
	@echo "  uninstall            Remove installed binary, man page, and completions from $(PREFIX)"
	@echo "  uninstall-user       Remove installed user files from $(HOME)/.local"
	@echo "  clean                Remove build directory and documentation artifacts"
	@echo "  distclean            Alias for clean"

build:
	$(CMAKE) -B $(BUILD_DIR) -G Ninja -DCMAKE_BUILD_TYPE=Debug
	$(CMAKE) --build $(BUILD_DIR)

release:
	$(CMAKE) -B $(BUILD_DIR) -G Ninja -DCMAKE_BUILD_TYPE=Release
	$(CMAKE) --build $(BUILD_DIR)
	$(STRIP) --strip-unneeded $(BUILD_DIR)/grm

clean:
	rm -rf $(BUILD_DIR)
	$(MAKE) -C docs clean BUILD_DIR="$(abspath $(BUILD_DIR))"

distclean: clean

format:
	$(CLANG_FORMAT) -i include/grm/*.hpp src/*.cpp tests/*.cpp

lint: release doc-check
	$(CLANG_TIDY) -p $(BUILD_DIR) src/*.cpp tests/*.cpp

analyze:
	$(SCAN_BUILD) --status-bugs $(CMAKE) --build $(BUILD_DIR) --clean-first

check: release
	$(CTEST) --test-dir $(BUILD_DIR) --output-on-failure

man:
	$(MAKE) -C docs man BUILD_DIR="$(abspath $(BUILD_DIR))"

doc-check:
	$(MAKE) -C docs doc-check

install-hooks:
	chmod +x .githooks/pre-push.bash
	git config core.hooksPath .githooks

hooks: install-hooks

install: release install-man install-completions
	install -d $(DESTDIR)$(BINDIR)
	install -m 0755 $(BUILD_DIR)/grm $(DESTDIR)$(BINDIR)/grm

install-user: release install-user-man install-user-completions
	install -d $(USER_BIN)
	install -m 0755 $(BUILD_DIR)/grm $(USER_BIN)/grm

install-man:
	$(MAKE) -C docs install-man BUILD_DIR="$(abspath $(BUILD_DIR))" PREFIX="$(PREFIX)" MAN_DIR="$(MAN1DIR)" DESTDIR="$(DESTDIR)"

install-user-man:
	$(MAKE) -C docs install-user-man BUILD_DIR="$(abspath $(BUILD_DIR))" USER_MAN_DIR="$(USER_MAN_DIR)"

install-completions:
	install -d $(DESTDIR)$(BASH_COMPLETION_DIR)
	install -m 0644 completions/grm.bash $(DESTDIR)$(BASH_COMPLETION_DIR)/grm

install-user-completions:
	install -d $(USER_BASH_COMPLETION_DIR)
	install -m 0644 completions/grm.bash $(USER_BASH_COMPLETION_DIR)/grm

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/grm
	rm -f $(DESTDIR)$(MAN1DIR)/grm.1
	rm -f $(DESTDIR)$(BASH_COMPLETION_DIR)/grm

uninstall-user:
	rm -f $(USER_BIN)/grm
	rm -f $(USER_MAN_DIR)/grm.1
	rm -f $(USER_BASH_COMPLETION_DIR)/grm
