CC = gcc
CFLAGS = -c -g -Wall

# Directorio de salida
OUT_DIR = out

EXECS = $(OUT_DIR)/clientGame $(OUT_DIR)/serverGame

all: $(OUT_DIR) $(OUT_DIR)/game.o $(EXECS)

# Crear la carpeta out si no existe
$(OUT_DIR):
	mkdir -p $(OUT_DIR)

$(OUT_DIR)/game.o: game.c
	$(CC) $(CFLAGS) game.c -o $(OUT_DIR)/game.o

$(OUT_DIR)/clientGame: $(OUT_DIR)/game.o $(OUT_DIR)/clientGame.o 
	$(CC) $(OUT_DIR)/game.o $(OUT_DIR)/clientGame.o -o $(OUT_DIR)/clientGame

$(OUT_DIR)/clientGame.o: clientGame.c
	$(CC) $(CFLAGS) clientGame.c -o $(OUT_DIR)/clientGame.o

$(OUT_DIR)/serverGame: $(OUT_DIR)/game.o $(OUT_DIR)/serverGame.o 
	$(CC) $(OUT_DIR)/game.o $(OUT_DIR)/serverGame.o -lpthread -o $(OUT_DIR)/serverGame

$(OUT_DIR)/serverGame.o: serverGame.c
	$(CC) $(CFLAGS) serverGame.c -o $(OUT_DIR)/serverGame.o
    
clean:
	rm -rf $(OUT_DIR)