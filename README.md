# ViSimpl - Multi-view analysis tools
(c) 2015-2016. GMRV / URJC

www.gmrv.es
gmrv@gmrv.es

## Introduction

ViSimpl consists of a set of analysis tools providing  different views for the
deep analysis of the brain simulation datasets. This project contains both
SimPart and StackViz, ready to be used with BlueConfig datasets among other
file formats.

## Dependencies

### Strong dependences:
* OpenGL
* GLEW
* Qt5Core
* Qt5Gui
* Qt5Widgets
* Qt5OpenGL
* Eigen3
* HDF5
* ReTo (*)
* SimIL (*)
* PReFr (*)

(*) Note: These dependencies will be automatically downloaded and compiled with
the project.

### Weak dependences
* OpenMP: multi-core functioning.
* ZeroEQ: library for collaboration between applications (*)
* Lexis: vocabulary for ZeroEQ
* GMRVLex: additional vocabulary for ZeroEQ

(*) Note: In order to connect applications one another, it is necessary to 
compile the project with ZeroEQ and its vocabulary libraries.

## Building

ViSimpl has been succesfully built and used on Ubuntu 14.04/16.04, Mac OSX
Yosemite and Windows 7/8 (Visual Studio 2013 Win64). The following steps
should be enough to build it:

```bash
git clone https://gitlab.gmrv.es/nsviz/visimpl.git ViSimpl
mkdir ViSimpl/build && cd ViSimpl/build
cmake .. -DCMAKE_BUILD_TYPE=Release
make
```
