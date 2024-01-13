CFILES:=$(shell find -L . -type f -name '*.c')
.PHONY: all clean

all: clean
	gcc $(CFILES) -o quintessence -Iinclude/

clean:
	rm -rf ./quintessence