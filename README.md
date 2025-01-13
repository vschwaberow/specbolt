# specbolt

Yet another ZX Spectrum Emulator. Part of a project for an upcoming C++ talk.


### Building

- install `conan` (version 2)
- do all the conan junk
- edit your default profile (or make another one) and point it at clang-19 etc: 

```
$ cat ~/.conan2/profiles/default 
[buildenv]
CC=/opt/compiler-explorer/clang-19.1.0/bin/clang
CXX=/opt/compiler-explorer/clang-19.1.0/bin/clang++
[settings]
arch=x86_64
build_type=Release
compiler=clang
compiler.cppstd=26
compiler.libcxx=libstdc++11
compiler.version=19
os=Linux
```

- `conan install . --build=missing`
- `conan install . --build=missing --settings=build_type=Debug`