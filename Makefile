OBJDIR=obj
SRCDIR=src

CC=gcc
LD=ld
CFLAGS=-std=c11 -D_POSIX_C_SOURCE=2
LDSTATIC = $(shell libpng-config --ldflags )

_OBJS = main.o

ifdef RELEASE
DEFS += -xSSE3 -O3 -DNDEBUG
else
DEFS += -g
endif

OBJS = $(patsubst %,$(OBJDIR)/%,$(_OBJS))

snescityeditor: $(OBJS)
	$(CC) -o $@ $^ $(LDSTATIC)

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(OBJDIR)
	$(CC) -c -o $@ $< $(CFLAGS) $(DEFS)

$(OBJDIR):
	mkdir $(OBJDIR)

clean:
	rm -rf $(OBJDIR)/*.o snescityeditor
