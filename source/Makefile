RM       = rm -f
CC       = gcc
COMMIT_INFO = commit.h

CFLAGS   = -O2 -Wall
ifeq ($(OS),Windows_NT)
  LDFLAGS += -largp
endif

OBJS     = main.o input.o assemble.o expr.o code.o command.o\
         macro.o func.o proc.o symbol.o output.o crc.o\
         mml.o nes.o

EXEDIR   = ..

COMMIT = $(shell git rev-parse --short HEAD)
COMMIT_DIRTY = $(shell git diff-index HEAD)
ifneq ($(COMMIT_DIRTY),)
  COMMIT += (dirty)
endif

ifeq ($(OS),Windows_NT)
  TARGNES  = $(EXEDIR)/nesasm.exe
else
  TARGNES  = $(EXEDIR)/nesasm
endif

#
#

all: $(TARGNES)
default: $(TARGNES)

# CLEAN
#

clean:
	$(RM) *.o 
ifeq ($(OS),Windows_NT)
	@if [ -f "$(TARGNES)" ]; then for f in `cygcheck "$(TARGNES)" | grep .dll | grep msys` ; do rm -f $(EXEDIR)/`basename "$$f"` ; done fi
endif
	$(RM) $(TARGNES)
	$(RM) $(COMMIT_INFO)

# ASSEMBLER
#

$(OBJS)  : defs.h externs.h protos.h
main.o   : inst.h vars.h $(COMMIT_INFO)
expr.o   : expr.h
nes.o    : nes.h
%.o : %.c
	$(CC) $(CFLAGS) -o $@ -c $<

.PHONY: $(COMMIT_INFO)
$(COMMIT_INFO):
	@echo "#define COMMIT \"$(COMMIT)\"" > $(COMMIT_INFO)

# EXE
#

$(TARGNES): $(OBJS)
	$(CC) -o $(TARGNES) $(OBJS) $(LDFLAGS)
ifeq ($(OS),Windows_NT)
	@for f in `cygcheck "$(TARGNES)" | grep .dll | grep msys` ; do cp -vf `cygpath "$$f"` $(EXEDIR)/ ; done
endif
