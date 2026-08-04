# GNUmakefile for grm (Telegram CLI Manager in C++23)

BUILD_DIR := build
PREFIX ?= /usr/local
INSTALL_BIN ?= $(PREFIX)/bin
USER_BIN ?= $(HOME)/bin
MAN_DIR ?= $(PREFIX)/share/man/man1
USER_MAN_DIR ?= $(HOME)/.local/share/man/man1

CMAKE := cmake
BUILD_TYPE ?= Debug

.PHONY: all build release clean check format lint analyze man doc-check install install-user install-man install-user-man install-completions install-user-completions

all: build

build:
	$(CMAKE) -B $(BUILD_DIR) -G Ninja -DCMAKE_BUILD_TYPE=Debug
	$(CMAKE) --build $(BUILD_DIR)

release:
	$(CMAKE) -B $(BUILD_DIR) -G Ninja -DCMAKE_BUILD_TYPE=Release
	$(CMAKE) --build $(BUILD_DIR)
	strip --strip-unneeded $(BUILD_DIR)/grm

clean:
	rm -rf $(BUILD_DIR)
	$(MAKE) -C docs clean BUILD_DIR="$(abspath $(BUILD_DIR))"

format:
	clang-format -i include/grm/*.hpp src/*.cpp

lint: release doc-check
	clang-tidy -p $(BUILD_DIR) src/*.cpp

analyze:
	scan-build --status-bugs $(CMAKE) --build $(BUILD_DIR) --clean-first

check: release
	ctest --test-dir $(BUILD_DIR) --output-on-failure

man:
	$(MAKE) -C docs man BUILD_DIR="$(abspath $(BUILD_DIR))"

doc-check:
	$(MAKE) -C docs doc-check

BASH_COMPLETION_DIR ?= $(PREFIX)/share/bash-completion/completions
USER_BASH_COMPLETION_DIR ?= $(HOME)/.local/share/bash-completion/completions

install: release install-man
	install -d $(DESTDIR)$(INSTALL_BIN)
	install -m 0755 $(BUILD_DIR)/grm $(DESTDIR)$(INSTALL_BIN)/grm

install-user: release install-user-man
	install -d $(USER_BIN)
	install -m 0755 $(BUILD_DIR)/grm $(USER_BIN)/grm

install-man:
	$(MAKE) -C docs install-man BUILD_DIR="$(abspath $(BUILD_DIR))" PREFIX="$(PREFIX)" MAN_DIR="$(MAN_DIR)" DESTDIR="$(DESTDIR)"

install-user-man:
	$(MAKE) -C docs install-user-man BUILD_DIR="$(abspath $(BUILD_DIR))" USER_MAN_DIR="$(USER_MAN_DIR)"

install-completions:
	install -d $(DESTDIR)$(BASH_COMPLETION_DIR)
	install -m 0644 completions/grm.bash $(DESTDIR)$(BASH_COMPLETION_DIR)/grm

install-user-completions:
	install -d $(USER_BASH_COMPLETION_DIR)
	install -m 0644 completions/grm.bash $(USER_BASH_COMPLETION_DIR)/grm
