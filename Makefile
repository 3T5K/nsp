.PHONY: help install uninstall dist

PROJECT := nsp
VERSION := 1.0.0
DIST    := $(PROJECT)-$(VERSION)
HEADER  := nsp.hpp
PREFIX  := /usr/local
INSDIR  := $(PREFIX)/include

help:
	@echo "[ TARGETS ]"
	@echo "|-> help      : show this help"
	@echo "|-> install   : install into '$(INSDIR)'"
	@echo "|-> uninstall : uninstall from '$(INSDIR)'"
	@echo "\-> dist      : create a source tarball"

install:
	install -Dm 644 $(HEADER) -t $(INSDIR)

uninstall:
	rm -f $(INSDIR)/$(HEADER)

dist:
	mkdir -p $(DIST)
	cp -f LICENSE nsp.hpp README.md Makefile $(DIST)
	tar -czf $(DIST).tar.gz $(DIST)
	rm -rf $(DIST)
