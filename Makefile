
POCS = $(patsubst %.c, %.exe, $(shell find '*.c'))

%.exe: %.c
	gcc -std=c99 -O0 -s -o $@ $^
	../../../ia32/bin/pin -t obj-ia32/readb4write.dll -- $@

test: $(POCS)
