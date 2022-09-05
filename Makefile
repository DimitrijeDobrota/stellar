NAME = engine
CC = gcc

CFLAGS = -I include
LDFLAGS = -lm

SRC = src
OBJ = obj
BINDIR = bin

SRCS := $(wildcard $(SRC)/*.c)
OBJS=$(patsubst $(SRC)/%.c, $(OBJ)/%.o, $(SRCS))
TRGT=$(BINDIR)/$(NAME)
	
ifeq ($(DEBUG),Y)
	LDFLAGS += -lciid
	CFLAGS += -Wall -Wextra -Werror -Wpedantic -fsanitize=address -ggdb
else
	LDFLAGS += -lcii
	CFLAGS += -Ofast -flto
endif


all: pre $(TRGT)

pre:
	@mkdir -p $(BINDIR)
	@mkdir -p $(OBJ)

$(TRGT): $(OBJS)
	$(CC) $^ $(CFLAGS) $(LDFLAGS) -o $@

$(OBJ)/%.o: $(SRC)/%.c
	$(CC) -c $< -o $@ $(CFLAGS) $(LDFLAGS)

clean:
	rm $(TRGT) $(OBJS)

.PHONY: all clean pre
