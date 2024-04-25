# ParserCombinators	

Creating parser combinators in modern C++.

Based on video: [Parser Combinators From Scratch](https://www.youtube.com/watch?v=6oQLRhw5Ah0&list=PLP29wDx6QmW5yfO1LAgO8kU3aQEj8SIrU)

## Build 
To use different version of compilators, set the following env variables before cmake run (should support C++ 23 standard)
```
export CC=/gcc-trunk/bin/gcc
export CXX=/gcc-trunk/bin/g++
```

Build commands:
```
cmake -S . -B build
cmake --build  build --config Release
cmake --build  build --config Debug
```

Binaries output directory:
```
build\Release\
build\Debug\
```
