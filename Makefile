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
TARGET = $(BIN_DIR)/my_engine

all: $(TARGET)

$(TARGET): $(OBJECTS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(TARGET)

distclean: clean
	rm -f $(TARGET)

run: $(TARGET)
	./$(TARGET)

rebuild: clean all


