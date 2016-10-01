OBJDIR=obj
SRCDIR=src

_CC=gcc
LD=ld
CFLAGS=-std=c11
DEFS=-D_POSIX_C_SOURCE=2 $(shell libpng-config --cflags)
LDFLAGS = $(shell libpng-config --ldflags )

_OBJS = pngmap.o main.o

ifdef RELEASE
DEFS += -xSSE3 -O3 -DNDEBUG
else
DEFS += -g
endif

ifdef WIN_UI
_OBJS += win_ui.o
DEFS += -DUSE_WIN_UI
CFLAGS += -I/usr/include/w32api
LDFLAGS = -mwindows $(shell libpng-config --static --ldflags)
endif

CC = $(PREFIX)$(_CC)
OBJS = $(patsubst %,$(OBJDIR)/%,$(_OBJS))

snescityeditor: $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(OBJDIR)
	$(CC) -c -o $@ $< $(CFLAGS) $(DEFS)

$(OBJDIR):
	mkdir $(OBJDIR)

clean:
	rm -rf $(OBJDIR)/*.o snescityeditor
