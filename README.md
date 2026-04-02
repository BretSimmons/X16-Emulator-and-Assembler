# X16

## Tracing

You can obtain a trace of execution by running 
```
./x16 -l objectfile
```

This generates a `log.txt` that contains the PC and instruction sequence that the emulator
executes.

Partial traces of `2048.obj` and `rogue.obj` from a working emulator are in the `trace` directory.

## Mac
To use valgrind, Mac users can run in a Linux environment by installing Docker, then running
```
    make run-on-docker
```

This will give a Linux environment in which valgrind can run.

When you are done with docker, you can run `make clean` to remove the docker image you downloaded.
