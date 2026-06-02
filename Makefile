CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11 -pedantic -Iinclude
AR      = ar

SRC     = src/categorical_agents.c
OBJ     = $(SRC:.c=.o)
LIB     = libcategorical_agents.a

.PHONY: all test clean

all: $(LIB)

$(LIB): $(OBJ)
	$(AR) rcs $@ $^

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

test: $(LIB) tests/test_categorical_agents.c
	$(CC) $(CFLAGS) -o tests/test_runner tests/test_categorical_agents.c -L. -lcategorical_agents
	./tests/test_runner

clean:
	rm -f $(OBJ) $(LIB) tests/test_runner
