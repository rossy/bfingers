CC=gcc
CFLAGS=-Wall -O0 -flto=jobserver -march=i686 -mtune=atom
LDFLAGS=-Ofast -s -flto=jobserver -march=i686 -mtune=atom -fuse-linker-plugin -fwhole-program
LIBS=-llua -llzma -lSDL -lSDLmain -lz -lpng -lrt -lGL
DEFS=-DCOMPILER="\"$(shell $(CC) --version|head -n1)\"" -DUNAME="\"$(shell uname -snrmpo)\""
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

#COLOR_RED=\e[1;31m
#COLOR_CLEAR=\e[0m

all: $(BINDIR)$(EXECUTABLE)

.PHONY: all clean distclean dist static cloc gar inifile libchash platform lua

static: PATH:=libs/bin:$(PATH)
static: CFLAGS+=-Ilibs/include
static: LDFLAGS+=-Llibs/lib
static: all

-include $(OBJECTS:.o=.d)

$(BINDIR):
	@mkdir -p $(BINDIR)

$(OBJDIR):
	@mkdir -p $(OBJDIR)

$(BINDIR)$(EXECUTABLE): $(OBJDIR)data.c $(OBJECTS) $(LIBCHASH_OBJECTS) $(GAR_OBJECTS) $(PLATFORM_OBJECTS) $(INIFILE_OBJECTS) | $(BINDIR)
	@echo -e "CCLD $@$(COLOR_RED)"
	+@$(CC) $(LDFLAGS) $(LIBS) $(OBJDIR)data.c $(OBJECTS) $(LIBCHASH_OBJECTS) $(GAR_OBJECTS) $(PLATFORM_OBJECTS) $(INIFILE_OBJECTS) -o $@
	@echo -ne "$(COLOR_CLEAR)"

$(OBJDIR)%.o: $(SRCDIR)%.c | $(OBJDIR)
	@echo -e "CC $<$(COLOR_RED)"
	+@$(CC) $(DEFS) $(CPPFLAGS) -MMD -MP -MT $@ -MF $(basename $@).d $(BFINGERS_CFLAGS) -c $< -o $@
	@echo -ne "$(COLOR_CLEAR)"

libchash: $(LIBCHASH_OBJECTS)

$(OBJDIR)%.o: $(SRCDIR)libchash/%.c | $(OBJDIR)
	@echo -e "CC $<$(COLOR_RED)"
	+@$(CC) $(DEFS) $(CPPFLAGS) -MMD -MP -MT $@ -MF $(basename $@).d $(LIBCHASH_CFLAGS) -c $< -o $@
	@echo -ne "$(COLOR_CLEAR)"

gar: $(GAR_OBJECTS)

$(OBJDIR)%.o: $(SRCDIR)gar/%.c | $(OBJDIR)
	@echo -e "CC $<$(COLOR_RED)"
	+@$(CC) $(DEFS) $(CPPFLAGS) -MMD -MP -MT $@ -MF $(basename $@).d $(GAR_CFLAGS) -c $< -o $@
	@echo -ne "$(COLOR_CLEAR)"

platform: $(PLATFORM_OBJECTS)

$(OBJDIR)%.o: $(SRCDIR)platform/%.c | $(OBJDIR)
	@echo -e "CC $<$(COLOR_RED)"
	+@$(CC) $(DEFS) $(CPPFLAGS) -MMD -MP -MT $@ -MF $(basename $@).d $(PLATFORM_CFLAGS) -c $< -o $@
	@echo -ne "$(COLOR_CLEAR)"

inifile: $(INIFILE_OBJECTS)

$(OBJDIR)%.o: $(SRCDIR)inifile/%.c | $(OBJDIR)
	@echo -e "CC $<$(COLOR_RED)"
	+@$(CC) $(DEFS) $(CPPFLAGS) -MMD -MP -MT $@ -MF $(basename $@).d $(INIFILE_CFLAGS) -c $< -o $@
	@echo -ne "$(COLOR_CLEAR)"

lua: $(OBJDIR)engine_init_lua.o

$(OBJDIR)data.c: $(SRCDIR)engine_init.lua | $(OBJDIR)
	@echo -e "DATA $<$(COLOR_RED)"
	@$(BIN2C) $< engine_init_lua > $(OBJDIR)data.c
	@echo -ne "$(COLOR_CLEAR)"

distclean: clean

dist: distclean
	tar cjvf bfingers-`date -u +%y%m%d`-src.tbz2 -C .. bfingers/src bfingers/Makefile bfingers/README bfingers/bin2c

clean:
	-@rm -f $(OBJDIR)data.c $(OBJDIR)*.o $(OBJDIR)*.d
	-@rm -f $(BINDIR)$(EXECUTABLE)
	-@( rmdir -p $(OBJDIR) || true ) 2> /dev/null > /dev/null
	-@( rmdir -p $(BINDIR) || true ) 2> /dev/null > /dev/null

cloc:
	@cloc src --exclude-dir=libchash
