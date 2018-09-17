CC = gcc
CFLAGS = -I ./include -Wall -std=c99
LDFLAGS = -L ./lib -lmingw32 -lsdl2main -lsdl2 -lopengl32 -lglew32 -lsimplex -lzio-utils -Wl,-subsystem,windows

SRCDIR = src
OBJDIR = obj
SRC = $(wildcard $(SRCDIR)/*.c)
OBJ	= $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRC))
$(info $(SRC))
$(info $(OBJ))
DEPENDFILE = .depend

yamc: $(OBJ)
	$(CC) $(CFLAGS) -o yamc $(OBJ) $(LDFLAGS)

dep: $(SRC)
	$(CC) -MM $(SRC) > $(DEPENDFILE)

-include $(DEPENDFILE)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f yamc
	rm -f $(OBJDIR)/*

