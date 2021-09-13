SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin

# Debug
SRC_DBG_DIR := src
OBJ_DBG_DIR := objD
BIN_DBG_DIR := binD

EXE := $(BIN_DIR)/Server
DBG_EXE := $(BIN_DBG_DIR)/ServerD
SRC := $(wildcard $(SRC_DIR)/*.c)
OBJ := $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
OBJ_DBG := $(SRC:$(SRC_DIR)/%.c=$(OBJ_DBG_DIR)/%.o)

CFLAGS   := -Wall
LDFLAGS  := -Llib
LDLIBS   := -pthread -lcrypto -lncurses

DBUGFLAGS := -Wall

.PHONY: all debug clean

all: $(EXE)

$(EXE): $(OBJ) | $(BIN_DIR)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@

clean:
	@$(RM) -rv $(BIN_DIR) $(OBJ_DIR)

-include $(OBJ:.o=.d)