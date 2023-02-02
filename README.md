# gr-carbon

![Version Number](https://img.shields.io/badge/Version-v0.1.0-blue)	[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)	![Docs](https://img.shields.io/badge/Docs-Doxygen-orange.svg)

This OOT Module contains dual source/sink blocks for use with the Carbon SDR.

## Dependencies

1.  GNU Radio 3.10
1.  gr-rwt
  
## Installation

### How to build gr-carbon
gr-carbon is built using cmake.

##### 0*. Initialize the Pybombs
If you are using a PyBombs workspace to run GNU Radio make sure to first run: `source setup_env.sh`

##### 1. Create the build directory
```bash
$ mkdir build
```
##### 2. Run CMake. It is recommended that gr-zwave be installed into a PyBombs workspace.
```bash
$ cd build/
$ cmake .. -DCMAKE_INSTALL_PREFIX=<PyBombs Workspace>
```

##### 3. Build and install.
```bash
$ make
$ make install
```

### How to remove gr-carbon
```bash
$ make uninstall
```
