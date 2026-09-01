# GNUmakefile for Container Operations (Podman Only)
SHELL := /bin/bash
.SHELLFLAGS := -euo pipefail -c
.DEFAULT_GOAL := build

IMAGE_NAME ?= localhost/grm:latest
CONTAINER_NAME ?= grm_dev
PODMAN ?= podman

.PHONY: all build run test stop clean help

all: build

build:
	@echo "==> Building rootless container image using Podman..."
	$(PODMAN) build -t $(IMAGE_NAME) -f Containerfile ..

run:
	@echo "==> Running container image with Podman..."
	$(PODMAN) run --rm -it --name $(CONTAINER_NAME) -v "$$(pwd)/../.grm:/home/grmuser/.config/grm:z" $(IMAGE_NAME)

test:
	@echo "==> Verifying container startup..."
	$(PODMAN) run --rm $(IMAGE_NAME) --version

stop:
	@echo "==> Stopping running container..."
	$(PODMAN) stop -t 2 $(CONTAINER_NAME) 2>/dev/null || true

clean:
	@echo "==> Removing container image..."
	$(PODMAN) rmi -f $(IMAGE_NAME) 2>/dev/null || true

help:
	@echo "Container GNUmakefile Targets:"
	@echo "  build - Build container image using Podman"
	@echo "  run   - Run container locally with volume mounts"
	@echo "  test  - Run basic version check inside container"
	@echo "  stop  - Stop active container"
	@echo "  clean - Remove container image"
