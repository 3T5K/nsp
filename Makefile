.PHONY: install uninstall

HEADER := nsp.hpp
PREFIX := /usr/local
INSDIR := $(PREFIX)/include

install:
	install -Dm 644 $(HEADER) -t $(INSDIR)

uninstall:
	rm -f $(INSDIR)/$(HEADER)
