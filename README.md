# nenuzhno engine

This repository contains the old discontinued codebase of my training engine/framework. I'm currently working on a new iteration of an engine, with better architecture (that's what i'm counting on).

I put it here so as not to lose by accident.

Writen in C++. Uses OpenGL/ES 2.

Use '-help' for comand line arguments.

## Dependencies

* [MinGW-W64](https://mingw-w64.org)
* [GLFW 3.3](https://www.glfw.org/)
* [GLM 0.9.9.0](https://glm.g-truc.net/0.9.9/index.html)
* [GLEW 2.0](http://glew.sourceforge.net/)

## Demos

**StarFleet** - space battles "game". With basic fighter AI, but withous gameplay

**tpsGame** - atempt to make real game. With bullet phisics, text map format, even menu

**VolLight** - Volumetric lighting

**gravity** - 2D simulation "game"

**skinning** - loads mdl files (Source engine) and renders some animation

**SSR** - kinda screen space reflections

**path_tracing** - uses glsl shader for ray trace

**blur-test** - atempt to do bokeh

**cube** - just wireframe cube

**ComprTex** - demonstrates some compressed texture formats

**[vbsp-gles](https://github.com/lewa-j/vbsp-gles)** - vbsp maps loader and renderer