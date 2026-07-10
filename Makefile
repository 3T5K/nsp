.PHONY: install uninstall

HEADER := nsp.hpp
INSDIR := /usr/local/include

install:
	install -Dm 644 $(HEADER) -t $(INSDIR)

uninstall:
	rm -f $(INSDIR)/$(HEADER)
