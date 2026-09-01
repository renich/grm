# GNUmakefile for grm (Group & Telegram Manager CLI)
# Copyleft (C) 2026 Rénich Bon Ćirić <renich@evalinux.com>

SHELL := /bin/bash
.SHELLFLAGS := -euo pipefail -c
.DEFAULT_GOAL := all

# Standard GNU Installation Directories (Compliant with GNU Coding Standards Section 7.2.5)
PREFIX ?= /usr/local
PREFIX_USER ?= $(HOME)/.local
EXEC_PREFIX ?= $(PREFIX)
BINDIR = $(EXEC_PREFIX)/bin
DATAROOTDIR = $(PREFIX)/share
MANDIR = $(DATAROOTDIR)/man
MAN1DIR = $(MANDIR)/man1
BASH_COMPLETION_DIR = $(DATAROOTDIR)/bash-completion/completions
DESTDIR ?=

# Build Toolchain & Directories
BUILD_DIR := build
CMAKE ?= cmake
CTEST ?= ctest
STRIP ?= strip
CLANG_FORMAT ?= clang-format
CLANG_TIDY ?= clang-tidy
SCAN_BUILD ?= scan-build
CRSTLINT ?= crstlint
PODMAN ?= podman

# Source and documentation discovery
SRC_FILES := $(shell find src include -type f \( -name '*.cpp' -o -name '*.hpp' \) 2>/dev/null)
TEST_FILES := $(shell find tests -type f -name '*.cpp' 2>/dev/null)
RST_FILES := $(wildcard *.rst) $(shell find docs -type f -name '*.rst' 2>/dev/null)

# Fallback search order: ~/.local first, then system paths
export PKG_CONFIG_PATH := $(PREFIX_USER)/lib64/pkgconfig:$(PREFIX_USER)/lib/pkgconfig:$(shell pkg-config --variable=pc_path pkg-config 2>/dev/null):$(PKG_CONFIG_PATH)

.PHONY: all build release static rpm srpm check test spec format lint analyze doc-check \
        asan tsan sanitize man coverage install install-user install-local uninstall uninstall-user \
        install-man install-completions containers-build containers-run containers-test \
        setup hooks clean distclean help tdlib-bootstrap

all: build

# Development build
build: $(BUILD_DIR)/grm

$(BUILD_DIR)/grm: $(SRC_FILES) CMakeLists.txt | $(BUILD_DIR)
	$(CMAKE) -B $(BUILD_DIR) -G Ninja -DCMAKE_BUILD_TYPE=Debug -DENABLE_ASAN=OFF -DENABLE_TSAN=OFF -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++
	$(CMAKE) --build $(BUILD_DIR)

# Production release build (stripped)
release: $(BUILD_DIR)/grm-release

$(BUILD_DIR)/grm-release: $(SRC_FILES) CMakeLists.txt | $(BUILD_DIR)
	$(CMAKE) -B $(BUILD_DIR) -G Ninja -DCMAKE_BUILD_TYPE=Release -DENABLE_ASAN=OFF -DENABLE_TSAN=OFF -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++
	$(CMAKE) --build $(BUILD_DIR)
	$(STRIP) --strip-unneeded $(BUILD_DIR)/grm
	@touch $@
	@echo "==> Built optimized release binary: $(BUILD_DIR)/grm"

# Static / containerized build via Podman
static:
	@echo "==> Building containerized release binary using Podman..."
	$(MAKE) -C containers build

# Local RPM build target using rpmbuild
rpm: $(SRC_FILES) packaging/grm.spec
	@echo "==> Building RPM package locally..."
	mkdir -p rpmbuild/{BUILD,RPMS,SOURCES,SPECS,SRPMS}
	tar --exclude='./rpmbuild' --exclude='./build' --exclude='./.git' -czf rpmbuild/SOURCES/grm-0.8.1.tar.gz .
	rpmbuild --define "_topdir $$(pwd)/rpmbuild" -ba packaging/grm.spec
	@echo "==> RPM build complete in rpmbuild/RPMS/"

srpm: $(SRC_FILES) packaging/grm.spec
	@echo "==> Building SRPM package locally..."
	mkdir -p rpmbuild/{BUILD,RPMS,SOURCES,SPECS,SRPMS}
	tar --exclude='./rpmbuild' --exclude='./build' --exclude='./.git' -czf rpmbuild/SOURCES/grm-0.8.1.tar.gz .
	rpmbuild --define "_topdir $$(pwd)/rpmbuild" -bs packaging/grm.spec
	@echo "==> SRPM build complete in rpmbuild/SRPMS/"

$(BUILD_DIR):
	@mkdir -p $@

# Testing targets
test: check
spec: check

check:
	$(CMAKE) -B $(BUILD_DIR) -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DENABLE_ASAN=OFF -DENABLE_TSAN=OFF
	$(CMAKE) --build $(BUILD_DIR)
	$(CTEST) --test-dir $(BUILD_DIR) --output-on-failure
	$(MAKE) doc-check

asan:
	$(CMAKE) -B $(BUILD_DIR) -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DENABLE_ASAN=ON -DENABLE_TSAN=OFF
	$(CMAKE) --build $(BUILD_DIR)
	LSAN_OPTIONS="suppressions=$(abspath sanitizers/lsan.supp)" $(CTEST) --test-dir $(BUILD_DIR) --output-on-failure

tsan:
	$(CMAKE) -B $(BUILD_DIR) -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DENABLE_TSAN=ON -DENABLE_ASAN=OFF
	$(CMAKE) --build $(BUILD_DIR)
	$(CTEST) --test-dir $(BUILD_DIR) --output-on-failure

sanitize: asan

# Code formatting & static analysis
format:
	$(CLANG_FORMAT) -i include/grm/*.hpp src/*.cpp tests/*.cpp

lint: release doc-check
	$(CLANG_TIDY) -p $(BUILD_DIR) src/*.cpp tests/*.cpp

analyze:
	$(SCAN_BUILD) --status-bugs $(CMAKE) --build $(BUILD_DIR) --clean-first

# Documentation
doc-check:
	$(MAKE) -C docs doc-check

man:
	$(MAKE) -C docs man BUILD_DIR="$(abspath $(BUILD_DIR))"

# Containers (Delegated to containers/GNUmakefile)
containers-build:
	$(MAKE) -C containers build

containers-run:
	$(MAKE) -C containers run

containers-test:
	$(MAKE) -C containers test

# Development lifecycle & hooks
hooks:
	@mkdir -p .githooks
	@chmod +x .githooks/* 2>/dev/null || true
	git config core.hooksPath .githooks || true

setup: hooks
	$(MAKE) format
	$(MAKE) check

tdlib-bootstrap:
	@echo "Bootstrapping latest TDLib into $(PREFIX_USER)..."
	@rm -rf /tmp/tdlib-bootstrap
	git clone --depth 1 https://github.com/tdlib/td.git /tmp/tdlib-bootstrap
	$(CMAKE) -B /tmp/tdlib-bootstrap/build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$(PREFIX_USER) -DTD_ENABLE_LTO=ON /tmp/tdlib-bootstrap
	$(CMAKE) --build /tmp/tdlib-bootstrap/build --target install
	@rm -rf /tmp/tdlib-bootstrap
	@echo "TDLib successfully installed to $(PREFIX_USER)"

# Installation
install: release install-man install-completions
	install -d $(DESTDIR)$(BINDIR)
	install -m 0755 $(BUILD_DIR)/grm $(DESTDIR)$(BINDIR)/grm

install-user:
	$(MAKE) install PREFIX="$(PREFIX_USER)"

install-local: install-user

install-man:
	$(MAKE) -C docs install-man BUILD_DIR="$(abspath $(BUILD_DIR))" PREFIX="$(PREFIX)" MAN_DIR="$(MAN1DIR)" DESTDIR="$(DESTDIR)"

install-completions: release
	install -d $(DESTDIR)$(BASH_COMPLETION_DIR)
	@if [ -x $(BUILD_DIR)/grm ]; then \
		$(BUILD_DIR)/grm completion bash > completions/grm.bash 2>/dev/null || true; \
	fi
	install -m 0644 completions/grm.bash $(DESTDIR)$(BASH_COMPLETION_DIR)/grm

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/grm
	rm -f $(DESTDIR)$(MAN1DIR)/grm.1
	rm -f $(DESTDIR)$(BASH_COMPLETION_DIR)/grm

uninstall-user:
	$(MAKE) uninstall PREFIX="$(PREFIX_USER)"

# Cleanup
clean:
	rm -rf $(BUILD_DIR) rpmbuild/
	$(MAKE) -C docs clean BUILD_DIR="$(abspath $(BUILD_DIR))"
	$(MAKE) -C containers clean

distclean: clean

help:
	@echo "grm - GNUmakefile Target Summary"
	@echo ""
	@echo "Build Targets:"
	@echo "  build                - Build debug binary in $(BUILD_DIR)/ (default)"
	@echo "  release              - Build optimized stripped production binary"
	@echo "  static               - Build containerized release binary using Podman"
	@echo "  rpm / srpm           - Build local RPM / SRPM package"
	@echo "  tdlib-bootstrap      - Clone, compile, and install latest TDLib into $(PREFIX_USER)"
	@echo ""
	@echo "Quality & Testing Targets:"
	@echo "  check / test         - Run full CTest automated test suite & doc checks"
	@echo "  asan                 - Build with AddressSanitizer & UBSan and run tests"
	@echo "  tsan                 - Build with ThreadSanitizer and run tests"
	@echo "  format               - Apply clang-format to src/, include/, tests/"
	@echo "  lint                 - Run clang-tidy static analyzer"
	@echo "  doc-check            - Verify reStructuredText docs with crstlint & rstcheck"
	@echo "  man                  - Compile reStructuredText man page to $(BUILD_DIR)/grm.1"
	@echo ""
	@echo "Container Targets:"
	@echo "  containers-build     - Build rootless Podman image"
	@echo "  containers-run       - Run containerized application"
	@echo "  containers-test      - Test container startup"
	@echo ""
	@echo "Installation Targets:"
	@echo "  install              - Install release binary, man page, completions to $(PREFIX)"
	@echo "  install-user / local - Install release binary, man page, completions to $(PREFIX_USER)"
	@echo "  setup / hooks        - Configure Git hooks and run checks"
	@echo "  clean / distclean    - Remove build outputs, rpmbuild, and doc artifacts"
