CFILES:=$(shell find -L . -type f -name '*.c')
.PHONY: all clean

all: clean
	gcc $(CFILES) -o quintessence -Iinclude/ -fno-stack-check

clean:
	rm -rf ./quintessence