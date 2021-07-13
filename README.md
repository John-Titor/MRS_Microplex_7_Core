# MRS Microplex 7 core firmware

This project serves as a starting point for writing your own application to run on the Microplex 7 family
without the overhead (or all of the functionality) of the MRS Developer Studio libraries.

## Protothreads (coroutines)

Protothreads are a lightweight cooperative multitasking technique. The firmware uses these as a way of keeping
the various parts of the system decoupled.

The implementation is a cut-down version of https://github.com/zserge/pt
