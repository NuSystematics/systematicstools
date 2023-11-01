# `systematicstools`

  This package provides a framework for writing, using, and interpreting the output of modular tools for propagating systematic uncertainties within an experimental simulation tool chain.

## Build

Requires ROOT v6+ and CMake v3.21+ (see [here](#fetch-cmake-3-21) for updating CMake hints)

Having checked out the repository in `/path/to/repo`:

```bash
cd /path/to/repo
mkdir build; cd build
cmake ..
make install -j $(nproc)
```

## Introduction

  Experimental physics is lousy with models. The 'event' signatures recorded in
  detectors are very often ambiguous and thus extensive simulation is needed to
  interpret what is observed. If these simulations are incorrect (they are),
  then our interpretation of the underlying physics will be similarly
  incorrect. To account for this, we make plausible variations of many
  components of the simulation and hope (assume) that somewhere within the high
  dimensional space of variations, is a model that describes nature well enough
  for our purposes. Because of the computing time involved with running a full
  simulation, it is often attractive to be able to make such variations to
  a set of simulated observations at 'analysis-', or interpretation-, time.
  Sometimes these variations can be exactly parameterized and thus the varied, simulated observations are statistically equivalent to having re-run the entire simulation. Sometimes they are approximate.

  This package aims to provide a framework for developing and using tools that provide these variations. The framework itself aims to contain no assumptions on the experiment or physics analysis being performed, these are left up to specific implementations that reside in dependent packages.

## Where to start

  For a short primer on the main moving parts within `systematicstools`, see [Moving parts](doc/MovingParts.md).

  For a description of the two levels of configuration file, with examples, see [Tool Configuration](doc/ToolConfiguration.md) and [Parameter Headers](doc/ParameterHeaders.md).

  For tips on how to begin writing a new systematic provider, and a short description of the ISystProviderTool abstract base class, see [Writing A Provider](doc/WritingAProvider.md).

  For an example, and recommended documentation structure, of an ISystProviderTool implementation, see [ExampleSystProvider](doc/ExampleSystProvider.md).

### Fetch CMake 3.21

If you just want a local build of cmake for this package and do not want to update
your system cmake, you can try the below recipe.

```
CMAKEVERSION=3.21.7
cd /path/to/repo
wget https://github.com/Kitware/CMake/archive/refs/tags/v${CMAKEVERSION}.tar.gz
tar -zxf v${CMAKEVERSION}.tar.gz
cd CMake-${CMAKEVERSION}
./bootstrap
make -j 8
export PATH=$(pwd)/bin:${PATH}
which cmake; cmake --version
```