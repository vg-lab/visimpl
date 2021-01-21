# ViSimpl - Multi-view analysis tools
(c) 2015-2020. GMRV / URJC

www.gmrv.es
gmrv@gmrv.es

## Introduction

ViSimpl consists of a set of analysis tools providing different views for the
deep analysis of the brain simulation datasets. This project contains both
SimPart and StackViz, ready to be used with BlueConfig datasets among other
file formats such as specific HDF5 and CSV. 
ViSimpl can represent and simulate data provided by [nest](https://github.com/nest)
connecting to a server that provides its REST API.   

## Known limitations

ViSimpl can only simulate data within 32 bits size and precision. 

## Dependencies

### Strong dependences:
* OpenGL
* GLEW
* Boost
* Eigen3
* HDF5
* Qt 5.X (Qt5Core, Qt5Gui, Qt5Widgets and Qt5OpenGL)
* GLM (https://glm.g-truc.net)
* ReTo (*)
* SimIL (*)
* PReFr (*)
* scoop (*)

(*) Note: These dependencies will be automatically downloaded and compiled with
the project.

### Weak dependences

* OpenMP: multi-core functioning.
* ZeroEQ: library for collaboration between applications (*)
* Lexis: vocabulary for ZeroEQ (*)
* GMRVLex: additional vocabulary for ZeroEQ (*)

(*) Note: In order to connect applications one another, it is necessary to 
compile the project with ZeroEQ and its vocabulary libraries (Lexis and GMRVLex).

## Building

ViSimpl has been succesfully built and used on Ubuntu 14.04/16.04, Mac OSX
Yosemite and Windows 7/8 (Visual Studio 2013 Win64). Please note that Brion
compatibility on Windows is limited, therefore ViSimpl is not guaranteed to
work with Brion in Windows systems. The following steps should be enough to build
 it:

```bash
git clone --recursive https://gitlab.gmrv.es/nsviz/visimpl.git ViSimpl
mkdir ViSimpl/build && cd ViSimpl/build
cmake .. -DCMAKE_BUILD_TYPE=Release -DCLONE_SUBPROJECTS=ON
make
```
