# ViSimpl - Multi-view analysis tools
(c) 2015-2021. GMRV / URJC

https://vg-lab.es/

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

ViSimpl has been succesfully built and used on Ubuntu 14.04/16.04 and Mac OSX
Yosemite. The following steps should be enough to build it:

```bash
git clone --recursive https://gitlab.gmrv.es/nsviz/visimpl.git ViSimpl
mkdir ViSimpl/build && cd ViSimpl/build
cmake .. -DCMAKE_BUILD_TYPE=Release -DCLONE_SUBPROJECTS=ON
make
```

## Attributions

This project has been made within the [URJC](https://urjc.es/) with the collaboration
and funding of the [HBP](https://www.humanbrainproject.eu/en/) european project, and 
it is included in its [EBrains](https://ebrains.eu/) research platform. 