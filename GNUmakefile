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

format:
	clang-format -i include/grm/*.hpp src/*.cpp

lint: release doc-check
	clang-tidy -p $(BUILD_DIR) src/*.cpp

analyze:
	scan-build --status-bugs $(CMAKE) --build $(BUILD_DIR) --clean-first

check: release
	ctest --test-dir $(BUILD_DIR) --output-on-failure

man:
	mkdir -p $(BUILD_DIR)
	rst2man docs/man/grm.1.rst $(BUILD_DIR)/grm.1

doc-check:
	rstcheck docs/user/grm.rst docs/man/grm.1.rst

BASH_COMPLETION_DIR ?= $(PREFIX)/share/bash-completion/completions
USER_BASH_COMPLETION_DIR ?= $(HOME)/.local/share/bash-completion/completions

install: release install-man
	install -d $(DESTDIR)$(INSTALL_BIN)
	install -m 0755 $(BUILD_DIR)/grm $(DESTDIR)$(INSTALL_BIN)/grm

install-user: release install-user-man
	install -d $(USER_BIN)
	install -m 0755 $(BUILD_DIR)/grm $(USER_BIN)/grm

install-man: man
	install -d $(DESTDIR)$(MAN_DIR)
	install -m 0644 $(BUILD_DIR)/grm.1 $(DESTDIR)$(MAN_DIR)/grm.1

install-user-man: man
	install -d $(USER_MAN_DIR)
	install -m 0644 $(BUILD_DIR)/grm.1 $(USER_MAN_DIR)/grm.1

install-completions:
	install -d $(DESTDIR)$(BASH_COMPLETION_DIR)
	install -m 0644 completions/grm.bash $(DESTDIR)$(BASH_COMPLETION_DIR)/grm

install-user-completions:
	install -d $(USER_BASH_COMPLETION_DIR)
	install -m 0644 completions/grm.bash $(USER_BASH_COMPLETION_DIR)/grm
