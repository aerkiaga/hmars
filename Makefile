CC = gcc
SRC_DIR = src
OBJ = main.o simulate.o load.o jit.o parser.o debug.o entropy.o
LIB = $(JIT_LIB) -pthread -lm
JIT_LIB = -L/usr/local/lib64 -ljit -Wl,-rpath=/usr/local/lib64
SDL_LIB = -L/usr/local/lib -lSDL2 -Wl,-rpath=/usr/local/lib
SDL_INC = -I/usr/local/include

hmars: CFLAGS = -Wall -O3 -g -march=native $(LIB) $(SRC)
hmars-gui: CFLAGS = -Wall -O3 -g -march=native -D_COREVIEW_ $(SDL_INC) $(SDL_LIB) $(LIB) $(SRC)

.PHONY: clean

%.o: $(SRC_DIR)/%.c
	$(CC) -c -o $@ $< $(CFLAGS)

all:
	$(MAKE) hmars
	$(MAKE) hmars-gui

hmars: $(OBJ)
	$(CC) -o hmars $(CFLAGS) $(OBJ)
	$(MAKE) clean
hmars-gui: $(OBJ)
	$(CC) -o hmars-gui $(CFLAGS) $(OBJ)
	$(MAKE) clean
clean:
	rm -f *.o
