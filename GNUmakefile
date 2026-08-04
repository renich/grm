# GNUmakefile for grm (Telegram CLI Manager in C++23)

BUILD_DIR := build
PREFIX ?= /usr/local
INSTALL_BIN ?= $(PREFIX)/bin
USER_BIN ?= $(HOME)/bin

CMAKE := cmake
BUILD_TYPE ?= Release

.PHONY: all build release clean check format install install-user

all: release

build:
	$(CMAKE) -B $(BUILD_DIR) -G Ninja -DCMAKE_BUILD_TYPE=$(BUILD_TYPE)
	$(CMAKE) --build $(BUILD_DIR)

release:
	$(CMAKE) -B $(BUILD_DIR) -G Ninja -DCMAKE_BUILD_TYPE=Release
	$(CMAKE) --build $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)

format:
	clang-format -i include/grm/*.hpp src/*.cpp

check: release

install: release
	install -d $(DESTDIR)$(INSTALL_BIN)
	install -m 0755 $(BUILD_DIR)/grm $(DESTDIR)$(INSTALL_BIN)/grm

install-user: release
	install -d $(USER_BIN)
	install -m 0755 $(BUILD_DIR)/grm $(USER_BIN)/grm
