# GNUmakefile for grm documentation & man pages

BUILD_DIR ?= $(CURDIR)/../build
PREFIX ?= /usr/local
MAN_DIR ?= $(PREFIX)/share/man/man1
USER_MAN_DIR ?= $(HOME)/.local/share/man/man1

RST2MAN ?= rst2man
RSTCHECK ?= rstcheck

.PHONY: all man doc-check clean install-man install-user-man

all: man

man:
	mkdir -p $(BUILD_DIR)
	$(RST2MAN) man/grm.1.rst $(BUILD_DIR)/grm.1
	sed -i 's/\.SH Name/\.SH NAME/' $(BUILD_DIR)/grm.1
	sed -i 's/\.SH Author/\.SH Authors/' $(BUILD_DIR)/grm.1

doc-check:
	$(RSTCHECK) user/grm.rst man/grm.1.rst adrs/*.rst

clean:
	rm -f $(BUILD_DIR)/grm.1

install-man: man
	install -d $(DESTDIR)$(MAN_DIR)
	install -m 0644 $(BUILD_DIR)/grm.1 $(DESTDIR)$(MAN_DIR)/grm.1

install-user-man: man
	install -d $(USER_MAN_DIR)
	install -m 0644 $(BUILD_DIR)/grm.1 $(USER_MAN_DIR)/grm.1
