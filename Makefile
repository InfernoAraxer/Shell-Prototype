OUTPUT = leak
CFLAGS = -g -Wall -Werror -std=c99 -fsanitize=address
LFLAGS = -lm

%: %.c %.h
	gcc $(CFLAGS) -o $@ $< $(LFLAGS)

%: %.c
	gcc $(CFLAGS) -o $@ $< $(LFLAGS)

all: $(OUTPUT)

clean:
	rm -f *.o $(OUTPUT)
