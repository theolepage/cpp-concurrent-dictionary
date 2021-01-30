# cpp-concurrent-dictionary

Concurrent dictionary with one implementation based on concurrent hash maps and one based on concurrent tries.

## Structure

- `hashmap_implementation` contains the hashmap implementation
- `trie_implementation` contains the trie implementation
- `async_implementation` contains the async implementation
- `fusion_implementation` contains the implementation of our trie structure using our hashmap

## Compilation

1. Create your build directory.

```
mkdir build && cd build
```

2. Install project dependancies with the [conan](https://docs.conan.io/en/latest/introduction.html) package manager.

```
conan profile update settings.compiler.libcxx=libstdc++11 default
conan install .. --build missing
```

3. Build the project (in Debug or Release) with CMake.

```
cmake .. -DCMAKE_BUILD_TYPE=Debug
```

or

```
cmake .. -DCMAKE_BUILD_TYPE=Release
```

## Usage

- Use `./tests` to run tests
- Use `./bench` to run benchmarks