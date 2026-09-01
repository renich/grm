# GNUmakefile for Sphinx Documentation & Manual Pages
SHELL := /bin/bash
.SHELLFLAGS := -euo pipefail -c

SPHINXBUILD ?= sphinx-build
SPHINXAUTOBUILD ?= sphinx-autobuild
CRSTLINT ?= crstlint
RSTCHECK ?= rstcheck
RST2MAN ?= rst2man
SOURCEDIR := .
BUILDDIR := build/dirhtml
BUILD_DIR ?= $(CURDIR)/../build
PREFIX ?= /usr/local
MAN_DIR ?= $(PREFIX)/share/man/man1
USER_MAN_DIR ?= $(HOME)/.local/share/man/man1
DESTDIR ?=

.DEFAULT_GOAL := all

.PHONY: all dirhtml html dev check man doc-check clean install-man install-user-man help

all: dirhtml man

# Build clean directory HTML documentation using Sphinx dirhtml builder
dirhtml: $(BUILDDIR)/index.html

$(BUILDDIR)/index.html: $(shell find . -maxdepth 3 -name '*.rst' -o -name '*.py' 2>/dev/null)
	@echo "==> Building Sphinx documentation with dirhtml builder..."
	@mkdir -p $(BUILDDIR)
	@if command -v $(SPHINXBUILD) >/dev/null 2>&1; then \
		$(SPHINXBUILD) -b dirhtml "$(SOURCEDIR)" "$(BUILDDIR)"; \
	else \
		echo "Warning: sphinx-build not found in PATH; skipping HTML docs generation."; \
	fi

# Alternative standard single-directory HTML builder target
html:
	@echo "==> Building Sphinx documentation with html builder..."
	@mkdir -p build/html
	@if command -v $(SPHINXBUILD) >/dev/null 2>&1; then \
		$(SPHINXBUILD) -b html "$(SOURCEDIR)" "build/html"; \
	else \
		echo "Warning: sphinx-build not found in PATH; skipping HTML docs generation."; \
	fi

# Development live-reload server using sphinx-autobuild
dev:
	@echo "==> Starting sphinx-autobuild live reload server..."
	$(SPHINXAUTOBUILD) -b dirhtml "$(SOURCEDIR)" "$(BUILDDIR)"

# Manual page generation
man:
	@mkdir -p $(BUILD_DIR)
	@if command -v $(RST2MAN) >/dev/null 2>&1; then \
		$(RST2MAN) man/grm.1.rst $(BUILD_DIR)/grm.1; \
		sed -i 's/\.SH Name/\.SH NAME/' $(BUILD_DIR)/grm.1; \
		sed -i 's/\.SH Author/\.SH Authors/' $(BUILD_DIR)/grm.1; \
	fi

# Lint documentation files using crstlint and rstcheck
check: doc-check

doc-check:
	@echo "==> Linting reStructuredText documentation using crstlint..."
	@if command -v $(CRSTLINT) >/dev/null 2>&1; then \
		$(CRSTLINT) -r .; \
	else \
		echo "Warning: crstlint not found in PATH; skipping crstlint scan."; \
	fi
	@if command -v $(RSTCHECK) >/dev/null 2>&1; then \
		$(RSTCHECK) --config .rstcheck.cfg user/grm.rst man/grm.1.rst adrs/*.rst 2>/dev/null || true; \
	fi

install-man: man
	install -d $(DESTDIR)$(MAN_DIR)
	install -m 0644 $(BUILD_DIR)/grm.1 $(DESTDIR)$(MAN_DIR)/grm.1

install-user-man: man
	install -d $(USER_MAN_DIR)
	install -m 0644 $(BUILD_DIR)/grm.1 $(USER_MAN_DIR)/grm.1

clean:
	@echo "==> Cleaning generated documentation build artifacts..."
	rm -rf build/ $(BUILD_DIR)/grm.1

help:
	@echo "Documentation Build Targets:"
	@echo "  all             - Build dirhtml documentation and man pages (default)"
	@echo "  dirhtml         - Build clean directory HTML documentation in build/dirhtml/"
	@echo "  html            - Build standard HTML documentation in build/html/"
	@echo "  man             - Generate UNIX manual page grm.1"
	@echo "  check/doc-check - Run crstlint and rstcheck linters on documentation"
	@echo "  install-man     - Install man page to $(MAN_DIR)"
	@echo "  clean           - Remove generated documentation artifacts"
