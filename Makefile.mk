CC = gcc
CFLAGS = -Wall -g -std=c99 -Werror 

stats: main_program.o stats_functions.o
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c main_program.h stats_functions.h
	$(CC) $(CFLAGS) -c $<

.PHONY: clean
clean:
	rm -f *.o stats
.PHONY: help
help:
	@echo "stats:     Build the System Stats executable"
	@echo "clean:     Remove auto-generated files"
	@echo "help:      Display this help message"

# valgrind --show-leak-kinds=all --leak-check=full ./stats
