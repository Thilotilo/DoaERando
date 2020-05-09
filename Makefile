CC = g++

BINDIR = bin

DOAERANDO = DoaERando.exe
$(DOAERANDO)_OBJS = Rom.cpp Randomizer.cpp main.cpp
TESTFILES += $(DOAERANDO)

all: $(TESTFILES)

$(TESTFILES): | odir

odir:
	@mkdir -p $(BINDIR)

$(TESTFILES): $($(TESTFILES)_OBJS)
	$(CC) -g $($@_OBJS) $($@_DEFINES) -std=c++11 -o $(BINDIR)/$@
