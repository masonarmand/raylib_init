WARNINGS = -Wpedantic -pedantic-errors
WARNINGS += -Werror
WARNINGS += -Wall
WARNINGS += -Wextra
WARNINGS += -Wold-style-definition
WARNINGS += -Wcast-align
WARNINGS += -Wlogical-op
WARNINGS += -Wmissing-declarations
WARNINGS += -Wmissing-include-dirs
WARNINGS += -Wmissing-prototypes
WARNINGS += -Wnested-externs
WARNINGS += -Wpointer-arith
WARNINGS += -Wredundant-decls
WARNINGS += -Wsequence-point
WARNINGS += -Wshadow
WARNINGS += -Wstrict-prototypes
WARNINGS += -Wundef
WARNINGS += -Wunreachable-code
WARNINGS += -Wwrite-strings
WARNINGS += -Wdisabled-optimization
WARNINGS += -Wunsafe-loop-optimizations
WARNINGS += -Wfree-nonheap-object
WARNINGS += -Wswitch

CC = gcc
CFLAGS = -std=c89 -D_GNU_SOURCE
LDFLAGS =
SOURCES = raylib_init.c
EXEC = raylib-init

.PHONY: all install clean

all: build

build:
	$(CC) -g $(CFLAGS) $(WARNINGS) $(SOURCES) $(LDFLAGS) -o $(EXEC)

run:
	./$(EXEC)

install: all
	install -m 755 $(EXEC) /usr/bin

clean:
	-rm $(EXEC)
