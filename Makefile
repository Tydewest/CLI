CFLAGS = -Wall -Wextra -O2 -Isrc
SRC = src/main.c src/commands.c
OBJ = $(SRC:.c=.o)
OUT = dynaserve

all: $(OUT)

$(OUT): $(OBJ)
	$(CC) $(OBJ) -o $(OUT)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(OUT)