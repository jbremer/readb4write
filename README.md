readb4write
===========

Pintool to detect Read before Write memory access.

This Pintool demonstrates a Valgrind-like (although with less information)
way to locate uninitialized memory access (which often leads to undefined
behavior.) It supports both stack variables and heap objects. See the attached
Proof of Concept applications for more examples.

install
=======

Clone this Git repository in `$PIN/source/tools/`, launch `Visual Studio
Command Prompt (2010)`, cd to `$PIN/source/tools/readb4write`, run `..\nmake`.
Now, after building the Pintool, run `make` to build and execute the Proof of
Concept applications.

example run
===========

Example run when testing the Pintool against the three Proof of Concept
applications.

```
gcc -std=c99 -O0 -o poc1.exe poc1.c
../../../ia32/bin/pin -t obj-ia32/readb4write.dll -- poc1.exe
untainted address 0x0027ff1c is being read @ 0x004013c5..
a: 2130567168

gcc -std=c99 -O0 -o poc2.exe poc2.c
../../../ia32/bin/pin -t obj-ia32/readb4write.dll -- poc2.exe
untainted address 0x0027ff10 is being read @ 0x004013c6..
untainted address 0x0027ff1c is being read @ 0x004013dd..

gcc -std=c99 -O0 -o poc3.exe poc3.c
../../../ia32/bin/pin -t obj-ia32/readb4write.dll -- poc3.exe
untainted address 0x02792bd0 is being read @ 0x004013e6..
a0: 41490040
a1: 0
untainted address 0x02792bd8 is being read @ 0x00401418..
a2: 0
```

The *untainted address* log statements refer to the following
read-before-write bugs;

Proof of Concept 1:

- uninitialized variable `a` in `main`

Proof of Concept 2:

- uninitialized member `ctx->arr[15]` in `process`
- uninitialized member `ctx->c` in `process2`

Proof of Concept 3:

- uninitialized member `a[0]` in `main`
- uninitialized member `a[2]` in `main`
