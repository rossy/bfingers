CC=gcc
CFLAGS=-Wall -O0 -flto=jobserver -march=i686 -mtune=atom
LDFLAGS=-Ofast -s -flto=jobserver -march=i686 -mtune=atom -fuse-linker-plugin -fwhole-program
LIBS=-llua -llzma -lSDL -lSDLmain -lz -lpng -lrt -lGL
DEFS=-DCOMPILER="\"`$(CC) --version|head -n1`\"" -DUNAME="\"`uname -snrmpo`\""
BIN2C=./bin2c

ifeq ($(MSYSTEM),MINGW32)
LDFLAGS+=-mwindows
else
LIBS+=-pthread
endif

SRCDIR=src/
BINDIR=bin/
OBJDIR=obj/
EXECUTABLE=bfingers

SOURCES=$(addprefix $(SRCDIR), bfingers.c image.c log.c object.c script.c settings.c)
OBJECTS=$(addprefix $(OBJDIR), $(notdir $(SOURCES:.c=.o)))
BFINGERS_CFLAGS=-I$(SRCDIR)gar/ -I$(SRCDIR)libchash/ -I$(SRCDIR)platform/ $(CFLAGS)

LIBCHASH_SOURCES=$(addprefix $(SRCDIR), libchash/libchash.c)
LIBCHASH_OBJECTS=$(addprefix $(OBJDIR), $(notdir $(LIBCHASH_SOURCES:.c=.o)))
LIBCHASH_CFLAGS=-w -O0 -flto=jobserver

GAR_SOURCES=$(addprefix $(SRCDIR), gar/gar_map.c)
GAR_OBJECTS=$(addprefix $(OBJDIR), $(notdir $(GAR_SOURCES:.c=.o)))
GAR_CFLAGS=-I$(SRCDIR)libchash/ -I$(SRCDIR)platform/ $(CFLAGS)

PLATFORM_SOURCES=$(addprefix $(SRCDIR), platform/file.c platform/memory.c platform/thread.c)
PLATFORM_OBJECTS=$(addprefix $(OBJDIR), $(notdir $(PLATFORM_SOURCES:.c=.o)))
PLATFORM_CFLAGS=-I$(SRCDIR)gar/ -I$(SRCDIR)libchash/ $(CFLAGS)

INIFILE_SOURCES=$(addprefix $(SRCDIR), inifile/inifile_reader.c)
INIFILE_OBJECTS=$(addprefix $(OBJDIR), $(notdir $(INIFILE_SOURCES:.c=.o)))
INIFILE_CFLAGS=$(CFLAGS)

all: $(BINDIR)$(EXECUTABLE)

.PHONY: all clean static cloc gar inifile libchash platform lua

static: PATH:=libs/bin:$(PATH)
static: CFLAGS+=-Ilibs/include
static: LDFLAGS+=-Llibs/lib
static: all

-include $(OBJECTS:.o=.d)

$(BINDIR):
	mkdir -p $(BINDIR)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(BINDIR)$(EXECUTABLE): $(OBJDIR)engine_init_lua.o $(OBJECTS) $(LIBCHASH_OBJECTS) $(GAR_OBJECTS) $(PLATFORM_OBJECTS) $(INIFILE_OBJECTS) | $(BINDIR)
	+$(CC) $(LDFLAGS) $(LIBS) $(OBJDIR)engine_init_lua.o $(OBJECTS) $(LIBCHASH_OBJECTS) $(GAR_OBJECTS) $(PLATFORM_OBJECTS) $(INIFILE_OBJECTS) -o $@

$(OBJDIR)%.o: $(SRCDIR)%.c | $(OBJDIR)
	+$(CC) $(DEFS) $(CPPFLAGS) -MMD -MP -MT $@ -MF $(basename $@).d $(BFINGERS_CFLAGS) -c $< -o $@

libchash: $(LIBCHASH_OBJECTS)

$(OBJDIR)%.o: $(SRCDIR)libchash/%.c | $(OBJDIR)
	+$(CC) $(DEFS) $(CPPFLAGS) -MMD -MP -MT $@ -MF $(basename $@).d $(LIBCHASH_CFLAGS) -c $< -o $@

gar: $(GAR_OBJECTS)

$(OBJDIR)%.o: $(SRCDIR)gar/%.c | $(OBJDIR)
	+$(CC) $(DEFS) $(CPPFLAGS) -MMD -MP -MT $@ -MF $(basename $@).d $(GAR_CFLAGS) -c $< -o $@

platform: $(PLATFORM_OBJECTS)

$(OBJDIR)%.o: $(SRCDIR)platform/%.c | $(OBJDIR)
	+$(CC) $(DEFS) $(CPPFLAGS) -MMD -MP -MT $@ -MF $(basename $@).d $(PLATFORM_CFLAGS) -c $< -o $@

inifile: $(INIFILE_OBJECTS)

$(OBJDIR)%.o: $(SRCDIR)inifile/%.c | $(OBJDIR)
	+$(CC) $(DEFS) $(CPPFLAGS) -MMD -MP -MT $@ -MF $(basename $@).d $(INIFILE_CFLAGS) -c $< -o $@

lua: $(OBJDIR)engine_init_lua.o

$(OBJDIR)engine_init_lua.o: $(SRCDIR)engine_init.lua | $(OBJDIR)
	$(BIN2C) $< init_lua | $(CC) -w -x c -c - -o $@

clean:
	-rm -f $(OBJDIR)*.o $(OBJDIR)*.d
	-rmdir -p $(OBJDIR) 2> /dev/null > /dev/null
	-rmdir -p $(BINDIR) 2> /dev/null > /dev/null

rebuild:
	$(MAKE) clean
	$(MAKE)

cloc:
	 cloc src --exclude-dir=libchash
