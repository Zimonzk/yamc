CC = gcc
CFLAGS = -I ./include -Wformat -Wimplicit-function-declaration -Werror=implicit-function-declaration -Wreturn-type -Werror=return-type -std=c99 -ggdb
ifeq ($(OS),Windows_NT)
LDFLAGS = -L ./lib -lmingw32 -lsdl2main -lsdl2 -lopengl32 -lglew32 -lsimplex -llists -llogger -lpng -lpthread -Wl,-subsystem,windows
else
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
LDFLAGS = -L ./lib -framework OpenGL -lGLEW -lsimplex -llists -llogger -lSDL2 -lpng -lpthread -lm
else
LDFLAGS = -L ./lib -lGL -lGLEW -lsimplex -llists -llogger -lSDL2 -lpng -lpthread -lm
endif
endif

SRCDIR = src
OBJDIR = obj
SRC = $(wildcard $(SRCDIR)/*.c)
OBJ = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRC))
$(info $(SRC))
$(info $(OBJ))
DEPENDFILE = .depend

yamc: $(OBJ) dep lib/liblists.a lib/liblogger.a lib/libsimplex.a
	$(CC) $(CFLAGS) -o yamc $(OBJ) $(LDFLAGS)

dep: $(SRC)
	$(CC) $(CFLAGS) -MM $(SRC) > $(DEPENDFILE)
	sed -i 's/..*\.o/$(OBJDIR)\/&/' $(DEPENDFILE)

-include $(DEPENDFILE)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

lib/liblists.a: lists_submodule/liblists.a
	cp lists_submodule/liblists.a lib/liblists.a 

lists_submodule/liblists.a: FORCE
	cd lists_submodule && make

lib/liblogger.a: logger_submodule/liblogger.a
	cp logger_submodule/liblogger.a lib/liblogger.a

logger_submodule/liblogger.a: FORCE
	cd logger_submodule && make

lib/libsimplex.a: simplex-noise_submodule/open-simplex-noise.o
	ar -rcs lib/libsimplex.a simplex-noise_submodule/open-simplex-noise.o

simplex-noise_submodule/open-simplex-noise.o: FORCE
	cd simplex-noise_submodule && make

FORCE:


.PHONY: clean
clean:
	rm -f yamc
	rm -f $(OBJDIR)/*
	rm -f .depend
