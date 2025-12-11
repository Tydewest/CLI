# Makefile for Dynaserve CLI

CC = cc
CFLAGS = -Wall -Wextra -O2 -Isrc
SRC = src/main.c src/commands.c
OUT = dynaserve

# Get current Git tag or fallback
VERSION := $(shell git describe --tags --abbrev=0 2>/dev/null || echo "0.0.0")

.PHONY: all clean install

all: $(OUT)

$(OUT): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(OUT)
	@echo "Dynaserve CLI built successfully (v$(VERSION))"

clean:
	rm -f $(OUT) src/*.o

install: $(OUT)
	# Copy binary to /usr/local/bin (sudo may be required)
	sudo cp $(OUT) /usr/local/bin/
	# Write version to ~/.dynaserve_version
	@echo "$(VERSION)" > $(HOME)/.dynaserve_version
	@echo "Dynaserve CLI installed at /usr/local/bin/$(OUT) with version $(VERSION)"