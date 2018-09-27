CC = gcc
CFLAGS = -I ./include -Wall -std=c99 -ggdb
LDFLAGS = -L ./lib -lsoil -lmingw32 -lsdl2main -lsdl2 -lopengl32 -lglew32 -lsimplex -lzio-utils -llists -Wl,-subsystem,windows

SRCDIR = src
OBJDIR = obj
SRC = $(wildcard $(SRCDIR)/*.c)
OBJ = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRC))
$(info $(SRC))
$(info $(OBJ))
DEPENDFILE = .depend

yamc: $(OBJ) dep
	$(CC) $(CFLAGS) -o yamc $(OBJ) $(LDFLAGS)

dep: $(SRC)
	$(CC) -MM $(CFLAGS) $(SRC) > $(DEPENDFILE)

-include $(DEPENDFILE)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f yamc
	rm -f $(OBJDIR)/*
	rm -f .depend
