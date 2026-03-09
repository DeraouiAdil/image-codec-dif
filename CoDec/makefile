CC = gcc
OBJ_DIR = obj
BIN_DIR = bin
INC_DIR = include
SRC_DIR = src

$(shell mkdir -p $(OBJ_DIR) $(BIN_DIR))

PFLAGS = -I$(INC_DIR) 

CFLAGS = -Wall -fPIC -g#g

LPATH = $(PWD)/$(BIN_DIR) 

LFLAGS = -lm -L$(LPATH) -lcodec -Wl,-rpath,$(LPATH)

LIB_TARGET = $(BIN_DIR)/libcodec.so
TARGET = $(BIN_DIR)/codec

LIB_OBJS = $(OBJ_DIR)/CoDec.o $(OBJ_DIR)/image.o $(OBJ_DIR)/diff.o $(OBJ_DIR)/bitstream.o

MAIN_OBJ = $(OBJ_DIR)/main.o

all: $(TARGET)

lib: $(LIB_TARGET)

$(TARGET): $(MAIN_OBJ) $(LIB_TARGET)
	$(CC) -o $@ $(MAIN_OBJ) $(LFLAGS)

$(LIB_TARGET): $(LIB_OBJS)
	$(CC) -shared -o $@ $(LIB_OBJS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(PFLAGS) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR) *.dif *.pnm *.pgm *.ppm

.PHONY: all lib clean