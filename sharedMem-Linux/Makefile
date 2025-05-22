# Makefile for building embedded application.
# by Brian Fraser

# Edit this file to compile extra C files into their own programs.
TARGETS = sharedMem-Linux


PUBDIR = $(HOME)/cmpt433/public/myApps

OUTDIR = $(PUBDIR)
CROSS_TOOL = aarch64-linux-gnu-
CC_CPP = $(CROSS_TOOL)g++
CC_C = $(CROSS_TOOL)gcc

CFLAGS = -Wall -g -std=c99 -D _POSIX_C_SOURCE=200809L -Werror -Wshadow


# Convert the targets to full-path executables to delete.
OUTPUTS= $(addprefix $(OUTDIR)/, $(TARGETS))

# all: $(TARGETS) PrimeThread
all: $(TARGETS)

$(TARGETS):
	$(CC_C) $(CFLAGS) $@.c -o $(OUTDIR)/$@ -lpthread

PrimeThread:
	make --directory=primeThread

clean:
	rm -f $(OUTPUTS)
