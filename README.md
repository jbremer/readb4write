readb4write
===========

Pintool to detect Read before Write memory access.

This Pintool demonstrates a Valgrind-like (although with less information)
way to trace uninitialized memory access. It supports both stack variables and
heap objects. See the attached Proof of Concept applications for more
information.

install
=======

Clone this Git repository in `$PIN/source/tools/`, launch `Visual Studio
Command Prompt (2010)`, cd to `$PIN/source/tools`, run `..\nmake`.
Now, after building the Pintool, run `make` to build and execute the Proof of
Concept applications.
