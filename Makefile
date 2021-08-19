CC = gcc
CCFLAGS = -Wall

SRCDIR = src
BINDIR = bin

SRCFILES = $(wildcard $(SRCDIR)/*.c)
OBJFILES = $(SRCFILES:$(SRCDIR)/%.c=$(BINDIR)/%.o)
EXECUTABLE = $(BINDIR)/grotto

all: $(EXECUTABLE)

debug: CCFLAGS += -O0 -g
debug: $(EXECUTABLE)

release: CCFLAGS += -O3
release: $(EXECUTABLE)

$(EXECUTABLE): $(OBJFILES)
	$(CC) $(CCFLAGS) -o $(EXECUTABLE) $(OBJFILES)

$(BINDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CCFLAGS) -c -o $@ $<

.phony: all debug release