-include Makefile.cfg

OBJDIR=obj
SRCDIR=src

_CC=gcc
LD=ld
CFLAGS = -std=c11 -Wall
DEFS += -D_POSIX_C_SOURCE=2 $(shell libpng-config --cflags)

ifdef STATIC
LDFLAGS = -static $(shell libpng-config --static --ldflags )
else
LDFLAGS = $(shell libpng-config --ldflags )
endif

_OBJS = sramcity.o cityedit.o pngmap.o main.o

ifdef PROFILE
CFLAGS += -pg
LDFLAGS += -pg
endif

ifdef RELEASE
DEFS += -xSSE3 -O3 -DNDEBUG
else
DEFS += -g -O0
endif

ifdef NESMODE
DEFS += -DNESMODE
endif


ifdef SDL_UI
_OBJS += sdl_ui.o ui_menu.o
DEFS += -DUSE_SDL_UI
CFLAGS += $(shell sdl2-config --cflags)

ifdef STATIC
LDFLAGS += $(shell sdl2-config --static-libs) -lSDL2_image
else
LDFLAGS += $(shell sdl2-config --libs) -lSDL2_image
endif

endif

CC = $(PREFIX)$(_CC)
OBJS = $(patsubst %,$(OBJDIR)/%,$(_OBJS))

install: snescityeditor

snescityeditor: $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) -c -o $@ $< $(CFLAGS) $(DEFS)

$(OBJDIR):
	mkdir $(OBJDIR)

clean:
	rm -rf $(OBJDIR)/*.o snescityeditor

.PHONY: clean install
