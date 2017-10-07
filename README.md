[![Build Status](https://travis-ci.org/Francesco149/tinynx.svg?branch=master)](https://travis-ci.org/Francesco149/tinynx)

tiny, single-file C89 implementation of the
[nx file format](http://nxformat.github.io/) with no third-party
dependencies

```nx.c``` is under 900 lines of code including comments and
around 600 lines of pure code. disabling bitmap support with
```NX_NOBITMAP``` shaves off an extra ~200 lines.

i recommend compiling with musl libc for even smaller executable
and memory footprint.

# library
drop nx.c into your project:

```shell
cd path/to/my/project
curl https://raw.githubusercontent.com/Francesco149/tinynx/master/nx.c > nx.c
```

read ```nx.c``` for examples and documentation of the api

# command line interface
![](https://i.imgur.com/Q8WL2Z2.gif)
[video demonstration](https://streamable.com/19vik)

download binaries from the releases section or build with
```./build``` (or ```.\build.bat``` on windows)

run ```./nx``` with no parameters to see a list of options
available

install the nx executable wherever you prefer (ideally somewhere
within your ```PATH``` like ```/usr/bin``` on linux).

you can symlink nx to names that end with the desired command name
to create shorter aliases (like busybox does):

```shell
nx cat /path/to/file.nx:/some/node
ln -s nx nxcat
nxcat /path/to/file.nx:/some/node
```

