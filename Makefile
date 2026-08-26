CC = gcc
CFLAGS = -O3 -march=native -mtune=native -flto \
         -fomit-frame-pointer -fno-stack-protector \
         -funroll-loops -fstrict-aliasing -ffast-math -fno-math-errno \
         -DNDEBUG
LDFLAGS = -flto

SRC_DIR = src
BUILD_DIR = build
BIN_DIR = .

SOURCES = $(wildcard $(SRC_DIR)/*.c)
OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

# Exclude perft.c (contains main) from engine build
ENGINE_OBJS = $(filter-out $(BUILD_DIR)/perft.o, $(OBJECTS))
# Exclude main.c (contains main) from perft build
PERFT_OBJS = $(filter-out $(BUILD_DIR)/main.o, $(OBJECTS))

TARGET_ENGINE = $(BIN_DIR)/my_engine
TARGET_PERFT  = $(BIN_DIR)/perft

all: $(TARGET_ENGINE) $(TARGET_PERFT)

$(TARGET_ENGINE): $(ENGINE_OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

$(TARGET_PERFT): $(PERFT_OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(TARGET_ENGINE) $(TARGET_PERFT)

run_engine: $(TARGET_ENGINE)
	./$(TARGET_ENGINE)

run_perft: $(TARGET_PERFT)
	./$(TARGET_PERFT)

rebuild: clean all
