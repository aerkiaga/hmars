CC = clang
SRC_DIR = src
OBJ = main.o simulate.o load.o jit.o parser.o debug.o entropy.o
LIB = $(JIT_LIB) -pthread -lm
JIT_LIB = -L/usr/local/lib -ljit -Wl,-rpath=/usr/local/lib
SDL_LIB = -lSDL2
SDL_INC = -I/usr/include/SDL2

hmars: CFLAGS = -Wall -O3 -g -march=native $(SRC)
hmars-gui: CFLAGS = -Wall -O3 -g -march=native -D_COREVIEW_ $(SDL_INC) $(SRC)
hmars: LDFLAGS = $(LIB) $(SRC)
hmars-gui: LDFLAGS = $(SDL_LIB) $(LIB) $(SRC)

.PHONY: clean

%.o: $(SRC_DIR)/%.c
	$(CC) -c -o $@ $< $(CFLAGS)

all:
	$(MAKE) hmars
	$(MAKE) hmars-gui

hmars: $(OBJ)
	$(CC) -o hmars $(LDFLAGS) $(OBJ)
	$(MAKE) clean
hmars-gui: $(OBJ)
	$(CC) -o hmars-gui $(LDFLAGS) $(OBJ)
	$(MAKE) clean
clean:
	rm -f *.o
