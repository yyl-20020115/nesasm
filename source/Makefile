RM       = rm -f
CC       = gcc

CFLAGS   = -O2 -Wall
ifeq ($(OS),Windows_NT)
  LDFLAGS += -largp
endif

OBJS     = main.o input.o assemble.o expr.o code.o command.o\
         macro.o func.o proc.o symbol.o output.o crc.o\
         mml.o nes.o

EXEDIR   = ..

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
	$(RM) $(TARGNES)

# ASSEMBLER
#

$(OBJS)  : defs.h externs.h protos.h
main.o   : inst.h vars.h
expr.o   : expr.h
nes.o    : nes.h
%.o : %.c
	$(CC) $(CFLAGS) -o $@ -c $<

# EXE
#

$(TARGNES) : $(OBJS)
	$(CC) -o $(TARGNES) $(OBJS) $(LDFLAGS)
