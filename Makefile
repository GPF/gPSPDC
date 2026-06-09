# Host SDL build delegates to x86/ (see x86/Makefile for sources).
.PHONY: all clean

all clean:
	$(MAKE) -C x86 $@
