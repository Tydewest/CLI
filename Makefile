# Makefile for Dynaserve CLI

TARGET=dynaserve
SRC=src/main.c src/commands.c
CFLAGS=-Wall -Wextra -O2 -Isrc
VERSION=1.0.0
VERSION_FILE=.dynaserve_version

all: $(TARGET)

clean:
	rm -f $(TARGET) src/*.o $(VERSION_FILE)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)
	@echo $(VERSION) > $(VERSION_FILE)