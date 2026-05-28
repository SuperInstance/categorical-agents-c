CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -pedantic -Iinclude
SRCS = src/object.c src/morphism.c src/category.c src/functor.c src/naturality.c
OBJS = $(SRCS:.c=.o)

.PHONY: all test clean

all: libcategorical_agents.a

libcategorical_agents.a: $(OBJS)
	ar rcs $@ $^

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

test: libcategorical_agents.a tests/test_categorical.c
	$(CC) $(CFLAGS) -o test_categorical tests/test_categorical.c -L. -lcategorical_agents
	./test_categorical

clean:
	rm -f src/*.o libcategorical_agents.a test_categorical
