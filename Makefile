.SUFFIXES:

SRCDIR = src
TESTDIR = test

.PHONY: all lightnet test clean info

all: lightnet test

lightnet:
	@(cd $(SRCDIR) && make)

test:
	@(cd $(TESTDIR) && make)

clean:
	@(cd $(SRCDIR) && make clean);\
	(cd $(TESTDIR) && make clean)

info:
	@echo "Available make targets:"
	@echo "  all: make lightnet and tests"
	@echo "  lightnet: make lightnet"
	@echo "  test: make tests"
	@echo "  clean: clean all object files"
	@echo "  info: show this infomation"
