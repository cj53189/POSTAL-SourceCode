# FPS Demo

This example is a minimal first‑person shooter prototype built from scratch using SDL2. It demonstrates basic player movement with a ray‑casting renderer for a simple 2D map. The code does not reuse the original POSTAL engine.

## Building

```
g++ main.cpp -std=c++17 -I../SDL2/include -L../SDL2/libs/linux-x86 -lSDL2 -o fps_demo
```

Adjust the SDL2 paths if necessary. On other platforms, link against the appropriate SDL2 library files.

Run `./fps_demo` to start the demo.
