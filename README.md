# find - a basic file search utility

## Goal
This is an educational project, aimed at understanding how to interact with
POSIX and Linux-specific APIs to retrieve file information.

## Features:
* Filter by wildcards, size ranges, inodes and link counts
* Print search results to stdout or pass to an executable

## Building
```
$ mkdir build
$ cd build
$ cmake ..
$ make
```

Requires a C++14 compiler.

## Usage
Please refer to `find --help` for details.

## Testing
Tested by hand on Linux 4.12.

## Copyright
Ilya Bizyaev, 2019 (<me@ilyabiz.com>)

Licensed under MIT terms.

