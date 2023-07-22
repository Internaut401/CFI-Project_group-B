# Coreutils
## With plugin
- Configuration command:
``` 
$ CC=<GCC path> \ 
    CXX=<GCC path> \
    CXXFLAGS='-g -fno-stack-protector -fplugin=<plugin_path>' \
    CFLAGS='-g -fno-stack-protector -fplugin=<plugin_path>' \
    LDFLAGS='-L.' LIBS='-l:libcfi_static.a' \
    ../configure --prefix=<install-dir> --enable-gcc-warnings=no
```

- Compilation:
```
$ time make
make 25,51s user 7,72s system 33,196 total
```
- Binaries size:
```
$ du -bk <install_folder>/bin
17768 kB
```

## Without plugin
- Configuration command:
```
$ CC=<GCC path> \
    CXX=<GCC path> \
    CXXFLAGS='-g' CFLAGS='-g' \
    ../configure --enable-gcc-warnings=no
```
- Compilation time:
```
$ time make
make 25,33s user 7,82s system 33,784 total
```
- Binaries size:
```
$ du -bk <install_folder>/bin
17304 kB
```

# FFMPEG
## With plugin
- Configuration command:
```
$ ../configure --prefix=<install_folder> \
    --cc=<GCC path> \
    --cxx=<GCC path> \ 
    --extra-cxxflags='-g -fno-stack-protector -fplugin=<plugin path>' \ 
    --extra-cflags='-g -fno-stack-protector -fplugin=<plugin path>' \
    --extra-ldflags='-L.' \
    --extra-libs='-l:libcfi_static.a' \
    --disable-optimizations --disable-stripping
```
- Compilation time:
```
$ time make 
make 233,11s user 34,54s system 5:11,14 total
```
- Binaries and libraries size:
```
$ du -bk <install_folder>/bin
221300 kB
$ du -bk <install_folder>/lib
133703 kB
```

## Without plugin
- Configuration command:
```
$ ../configure --prefix=<install_folder>\
    --cc=<GCC path> \
    --cxx=<GCC path> \ 
    --extra-cxxflags='-g' \ 
    --extra-cflags='-g' \
    --disable-optimizations --disable-stripping
```
- Compilation time:
```
$ time make 
make 223,89s user 32,73s system 4:21,20 total
```
- Binaries and libraries size:
```
$ du -bk <install_folder>/bin
218525 kB
$ du -bk <install_folder>/lib
131095 kB
```